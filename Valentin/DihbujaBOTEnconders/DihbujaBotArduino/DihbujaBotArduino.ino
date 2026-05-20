#include "WiFiEsp.h"
#include "WiFiEspUdp.h"

// --- CONFIGURACIÓN WI-FI ---
char ssid[] = "ROBOT_COMPETICION"; 
char pass[] = "12345678"; 
int status = WL_IDLE_STATUS;
unsigned int localPort = 8080;
WiFiEspUDP Udp;

// ========================================================
// === CONFIGURACIÓN DE ENCODERS (ODOMETRÍA 4x4) ==========
// ========================================================
// Delante Izquierda (FL) -> D2 y 37
#define PIN_ENC_FL_INT 2
#define PIN_ENC_FL_DIR 37

// Delante Derecha (FR) -> D3 y 36
#define PIN_ENC_FR_INT 3
#define PIN_ENC_FR_DIR 36

// Atrás Izquierda (BL) -> 20SDA y 39
#define PIN_ENC_BL_INT 20
#define PIN_ENC_BL_DIR 39

// Atrás Derecha (BR) -> 21SCL y 38
#define PIN_ENC_BR_INT 21
#define PIN_ENC_BR_DIR 38

// Contadores de pulsos en memoria volátil (ultrarrápida)
volatile long ticks_FL = 0;
volatile long ticks_FR = 0;
volatile long ticks_BL = 0;
volatile long ticks_BR = 0;
bool emergencia_activa = false;

// Rutinas ISR súper rápidas para contar vueltas
void contar_FL() { ticks_FL++; }
void contar_FR() { ticks_FR++; }
void contar_BL() { ticks_BL++; }
void contar_BR() { ticks_BR++; }

// ========================================================
// ============ MEMORIA VARIABLES EN TIEMPO REAL ==========
// ========================================================
int figura_a_dibujar = 1;
int tiempo_espera_esquinas = 500;

// Calibración de Encoders (Ticks por CM)
float tk_cm_frente_triangulo = 40.0;
float tk_cm_diagonal         = 55.0;
float tk_cm_frente_cuad_rect = 40.0;
float tk_cm_lateral          = 65.0;

// Parámetros de trayectorias
int vel_tri_adelante = 100;   float tri_adelante = 25.0;
int vel_tri_diag_der = 100;   float tri_diag_der = 35.0;   float angulo_tri_der = 0.60;
int vel_tri_diag_izq = 100;   float tri_diag_izq = 35.0;   float angulo_tri_izq = 0.60;

int vel_cuad_adelante = 75;   float cuad_adelante = 20.0;
int vel_cuad_derecha  = 75;   float cuad_derecha  = 20.0;  float deriva_cuad_der = 0.0;
int vel_cuad_atras    = 85;   float cuad_atras    = 20.0;
int vel_cuad_izquierda= 75;   float cuad_izquierda= 20.0;  float deriva_cuad_izq = 0.0;

int vel_rect_adelante = 75;   float rect_adelante = 28.0;
int vel_rect_derecha  = 75;   float rect_derecha  = 12.0;  float deriva_rect_der = 0.0;
int vel_rect_atras    = 85;   float rect_atras    = 28.0;
int vel_rect_izquierda= 75;   float rect_izquierda= 12.0;  float deriva_rect_izq = 0.0;

// --- PINES DE LOS MOTORES (DRIVER L298N/OSOYOO) ---
#define speedPinR 9
#define RightMotorDirPin1 22
#define RightMotorDirPin2 24
#define LeftMotorDirPin1 26
#define LeftMotorDirPin2 28
#define speedPinL 10
#define speedPinRB 11
#define RightMotorDirPin1B 5
#define RightMotorDirPin2B 6
#define LeftMotorDirPin1B 7
#define LeftMotorDirPin2B 8
#define speedPinLB 12

void setup() {
  pinMode(RightMotorDirPin1, OUTPUT); pinMode(RightMotorDirPin2, OUTPUT); pinMode(speedPinL, OUTPUT);
  pinMode(LeftMotorDirPin1, OUTPUT);  pinMode(LeftMotorDirPin2, OUTPUT);  pinMode(speedPinR, OUTPUT);
  pinMode(RightMotorDirPin1B, OUTPUT);pinMode(RightMotorDirPin2B, OUTPUT);pinMode(speedPinLB, OUTPUT);
  pinMode(LeftMotorDirPin1B, OUTPUT); pinMode(LeftMotorDirPin2B, OUTPUT); pinMode(speedPinRB, OUTPUT);
  
  // Activar los 4 Encoders con resistencia Pull-Up interna de seguridad
  pinMode(PIN_ENC_FL_INT, INPUT_PULLUP); pinMode(PIN_ENC_FL_DIR, INPUT_PULLUP);
  pinMode(PIN_ENC_FR_INT, INPUT_PULLUP); pinMode(PIN_ENC_FR_DIR, INPUT_PULLUP);
  pinMode(PIN_ENC_BL_INT, INPUT_PULLUP); pinMode(PIN_ENC_BL_DIR, INPUT_PULLUP);
  pinMode(PIN_ENC_BR_INT, INPUT_PULLUP); pinMode(PIN_ENC_BR_DIR, INPUT_PULLUP);
  
  // Conectar las 4 interrupciones a sus rutinas
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_FL_INT), contar_FL, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_FR_INT), contar_FR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_BL_INT), contar_BL, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_BR_INT), contar_BR, CHANGE);

  stop_Stop();
  Serial.begin(115200);   
  Serial1.begin(115200);  

  WiFi.init(&Serial1);
  WiFi.beginAP(ssid, 10, pass, ENC_TYPE_WPA2_PSK);
  Udp.begin(localPort);
  Serial.println(F("Odometría Mecanum 4x4 lista. Esperando órdenes..."));
}

void loop() {
  revisarTelemetria();
}

void revisarTelemetria() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    char packetBuffer[256];
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = '\0';
      
      if (packetBuffer[0] == 'C') {
        char *token = strtok(packetBuffer, ",");
        if (token != NULL) {
          token = strtok(NULL, ","); if(token) figura_a_dibujar = atoi(token);
          token = strtok(NULL, ","); if(token) tk_cm_frente_triangulo = atof(token);
          token = strtok(NULL, ","); if(token) tk_cm_diagonal         = atof(token);
          token = strtok(NULL, ","); if(token) tk_cm_frente_cuad_rect = atof(token);
          token = strtok(NULL, ","); if(token) tk_cm_lateral          = atof(token);
          
          token = strtok(NULL, ","); if(token) vel_tri_adelante = atoi(token);
          token = strtok(NULL, ","); if(token) tri_adelante    = atof(token);
          token = strtok(NULL, ","); if(token) vel_tri_diag_der = atoi(token);
          token = strtok(NULL, ","); if(token) tri_diag_der    = atof(token);
          token = strtok(NULL, ","); if(token) angulo_tri_der  = atof(token);
          token = strtok(NULL, ","); if(token) vel_tri_diag_izq = atoi(token);
          token = strtok(NULL, ","); if(token) tri_diag_izq    = atof(token);
          token = strtok(NULL, ","); if(token) angulo_tri_izq  = atof(token);
          
          token = strtok(NULL, ","); if(token) vel_cuad_adelante = atoi(token);
          token = strtok(NULL, ","); if(token) cuad_adelante    = atof(token);
          token = strtok(NULL, ","); if(token) vel_cuad_derecha  = atoi(token);
          token = strtok(NULL, ","); if(token) cuad_derecha     = atof(token);
          token = strtok(NULL, ","); if(token) deriva_cuad_der  = atof(token);
          token = strtok(NULL, ","); if(token) vel_cuad_atras    = atoi(token);
          token = strtok(NULL, ","); if(token) cuad_atras       = atof(token);
          token = strtok(NULL, ","); if(token) vel_cuad_izquierda= atoi(token);
          token = strtok(NULL, ","); if(token) cuad_izquierda   = atof(token);
          token = strtok(NULL, ","); if(token) deriva_cuad_izq  = atof(token);
          
          token = strtok(NULL, ","); if(token) vel_rect_adelante = atoi(token);
          token = strtok(NULL, ","); if(token) rect_adelante    = atof(token);
          token = strtok(NULL, ","); if(token) vel_rect_derecha  = atoi(token);
          token = strtok(NULL, ","); if(token) rect_derecha     = atof(token);
          token = strtok(NULL, ","); if(token) deriva_rect_der  = atof(token);
          token = strtok(NULL, ","); if(token) vel_rect_atras    = atoi(token);
          token = strtok(NULL, ","); if(token) rect_atras       = atof(token);
          token = strtok(NULL, ","); if(token) vel_rect_izquierda= atoi(token);
          token = strtok(NULL, ","); if(token) rect_izquierda   = atof(token);
          token = strtok(NULL, ","); if(token) deriva_rect_izq  = atof(token);
          token = strtok(NULL, ","); if(token) tiempo_espera_esquinas = atoi(token);
          Serial.println(F("Base de odometría actualizada."));
        }
      } 
      else if (packetBuffer[0] == 'G') {
        emergencia_activa = false;
        if (figura_a_dibujar == 1)      { dibujarTriangulo(); } 
        else if (figura_a_dibujar == 2) { dibujarCuadrado(); } 
        else if (figura_a_dibujar == 3) { dibujarRectangulo(); }
      }
      else if (packetBuffer[0] == 'S') {
        emergencia_activa = true;
        stop_Stop();
      }
    }
  }
}

// ========================================================
// === EL CORAZÓN DEL SISTEMA: NAVEGACIÓN 4x4 POR ODOMETRÍA 
// ========================================================
void avanzarHastaTicks(long target_ticks) {
  // Ponemos a 0 los 4 contadores antes de empezar a movernos
  ticks_FL = 0;
  ticks_FR = 0;
  ticks_BL = 0;
  ticks_BR = 0;
  
  while (true) {
    revisarTelemetria(); // Permite recibir el botón de freno en pleno movimiento
    if (emergencia_activa) break;

    // Con las 4 ruedas leyendo, buscamos la que más haya girado.
    // Esto es magia para Mecanum: si derrapas o vas en diagonal, una rueda puede 
    // quedarse quieta o girar en vacío. El valor máximo nos da la distancia real empujada.
    long max_delanteras = max(ticks_FL, ticks_FR);
    long max_traseras   = max(ticks_BL, ticks_BR);
    long max_total      = max(max_delanteras, max_traseras);
    
    if (max_total >= target_ticks) {
      break; // ¡Distancia conseguida! Salimos del bucle.
    }
  }
}

// ========================================================
// ============ RUTINAS CINEMÁTICAS HOLONÓMICAS ===========
// ========================================================

void dibujarTriangulo() {
  long tk1 = (long)(tri_adelante * tk_cm_frente_triangulo);
  setMotors(vel_tri_adelante, vel_tri_adelante, vel_tri_adelante, vel_tri_adelante);
  avanzarHastaTicks(tk1); 
  if(emergencia_activa) return;
  if(tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  long tk2 = (long)(tri_diag_der * tk_cm_diagonal);
  int v2_suave = vel_tri_diag_der * angulo_tri_der;
  setMotors(v2_suave, -vel_tri_diag_der, -vel_tri_diag_der, v2_suave);
  avanzarHastaTicks(tk2); 
  if(emergencia_activa) return;
  if(tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  long tk3 = (long)(tri_diag_izq * tk_cm_diagonal);
  int v3_suave = vel_tri_diag_izq * angulo_tri_izq;
  setMotors(-vel_tri_diag_izq, v3_suave, v3_suave, -vel_tri_diag_izq);
  avanzarHastaTicks(tk3); 
  stop_Stop();
}

void dibujarCuadrado() {
  long tk_f = (long)(cuad_adelante * tk_cm_frente_cuad_rect);
  setMotors(vel_cuad_adelante, vel_cuad_adelante, vel_cuad_adelante, vel_cuad_adelante);
  avanzarHastaTicks(tk_f);
  if(emergencia_activa) return;
  if(tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  long tk_d = (long)(cuad_derecha * tk_cm_lateral);
  int v_d = vel_cuad_derecha; int d_d = deriva_cuad_der;
  setMotors(v_d + d_d, -v_d + d_d, -v_d + d_d, v_d + d_d);
  avanzarHastaTicks(tk_d); 
  if(emergencia_activa) return;
  if(tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  long tk_at = (long)(cuad_atras * tk_cm_frente_cuad_rect);
  setMotors(-vel_cuad_atras, -vel_cuad_atras, -vel_cuad_atras, -vel_cuad_atras);
  avanzarHastaTicks(tk_at); 
  if(emergencia_activa) return;
  if(tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  long tk_i = (long)(cuad_izquierda * tk_cm_lateral);
  int v_i = vel_cuad_izquierda; int d_i = deriva_cuad_izq;
  setMotors(-v_i + d_i, v_i + d_i, v_i + d_i, -v_i + d_i);
  avanzarHastaTicks(tk_i); 
  stop_Stop();
}

void dibujarRectangulo() {
  long tk_f = (long)(rect_adelante * tk_cm_frente_cuad_rect);
  setMotors(vel_rect_adelante, vel_rect_adelante, vel_rect_adelante, vel_rect_adelante);
  avanzarHastaTicks(tk_f); 
  if(emergencia_activa) return;
  if(tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  long tk_d = (long)(rect_derecha * tk_cm_lateral);
  int v_d = vel_rect_derecha; int d_d = deriva_rect_der;
  setMotors(v_d + d_d, -v_d + d_d, -v_d + d_d, v_d + d_d);
  avanzarHastaTicks(tk_d); 
  if(emergencia_activa) return;
  if(tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  long tk_at = (long)(rect_atras * tk_cm_frente_cuad_rect);
  setMotors(-vel_rect_atras, -vel_rect_atras, -vel_rect_atras, -vel_rect_atras);
  avanzarHastaTicks(tk_at); 
  if(emergencia_activa) return;
  if(tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  long tk_i = (long)(rect_izquierda * tk_cm_lateral);
  int v_i = vel_rect_izquierda; int d_i = deriva_rect_izq;
  setMotors(-v_i + d_i, v_i + d_i, v_i + d_i, -v_i + d_i);
  avanzarHastaTicks(tk_i); 
  stop_Stop();
}

void setMotors(int FL, int FR, int RL, int RR) {
  if (FL >= 0) { digitalWrite(LeftMotorDirPin1, LOW);  digitalWrite(LeftMotorDirPin2, HIGH); analogWrite(speedPinL, FL); } 
  else         { digitalWrite(LeftMotorDirPin1, HIGH); digitalWrite(LeftMotorDirPin2, LOW);  analogWrite(speedPinL, -FL); }
  
  if (FR >= 0) { digitalWrite(RightMotorDirPin1, LOW);  digitalWrite(RightMotorDirPin2, HIGH); analogWrite(speedPinR, FR); } 
  else         { digitalWrite(RightMotorDirPin1, HIGH); digitalWrite(RightMotorDirPin2, LOW);  analogWrite(speedPinR, -FR); }
  
  if (RL >= 0) { digitalWrite(LeftMotorDirPin1B, LOW);  digitalWrite(LeftMotorDirPin2B, HIGH); analogWrite(speedPinLB, RL); } 
  else         { digitalWrite(LeftMotorDirPin1B, HIGH); digitalWrite(LeftMotorDirPin2B, LOW);  analogWrite(speedPinLB, -RL); }
  
  if (RR >= 0) { digitalWrite(RightMotorDirPin1B, LOW);  digitalWrite(RightMotorDirPin2B, HIGH); analogWrite(speedPinRB, RR); } 
  else         { digitalWrite(RightMotorDirPin1B, HIGH); digitalWrite(RightMotorDirPin2B, LOW);  analogWrite(speedPinRB, -RR); }
}

void stop_Stop() {
  setMotors(0, 0, 0, 0);
}