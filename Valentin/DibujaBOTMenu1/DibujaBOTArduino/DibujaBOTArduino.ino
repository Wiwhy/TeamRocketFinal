#include "WiFiEsp.h"
#include "WiFiEspUdp.h"

// --- CONFIGURACIÓN WI-FI ---
char ssid[] = "ROBOT_COMPETICION"; 
char pass[] = "12345678"; 
int status = WL_IDLE_STATUS;
unsigned int localPort = 8080;
WiFiEspUDP Udp;

// ========================================================
// ============ MEMORIA VARIABLES EN TIEMPO REAL ==========
// ========================================================
int figura_a_dibujar = 1;
int tiempo_espera_esquinas = 500;

// Calibración de fricción suelo
float ms_cm_frente_triangulo = 21.0;
float ms_cm_diagonal         = 35.0;
float ms_cm_frente_cuad_rect = 28.0;
float ms_cm_lateral          = 43.3;

// Parámetros de trayectorias: TRIÁNGULO
int vel_tri_adelante = 100;   float tri_adelante = 25.0;
int vel_tri_diag_der = 100;   float tri_diag_der = 35.0;   float angulo_tri_der = 0.60;
int vel_tri_diag_izq = 100;   float tri_diag_izq = 35.0;   float angulo_tri_izq = 0.60;

// Parámetros de trayectorias: CUADRADO
int vel_cuad_adelante = 75;   float cuad_adelante = 20.0;
int vel_cuad_derecha  = 75;   float cuad_derecha  = 20.0;  float deriva_cuad_der = 0.0;
int vel_cuad_atras    = 85;   float cuad_atras    = 20.0;
int vel_cuad_izquierda= 75;   float cuad_izquierda= 20.0;  float deriva_cuad_izq = 0.0;

// Parámetros de trayectorias: RECTÁNGULO
int vel_rect_adelante = 75;   float rect_adelante = 28.0;
int vel_rect_derecha  = 75;   float rect_derecha  = 12.0;  float deriva_rect_der = 0.0;
int vel_rect_atras    = 85;   float rect_atras    = 28.0;
int vel_rect_izquierda= 75;   float rect_izquierda= 12.0;  float deriva_rect_izq = 0.0;

// --- PINES DE LOS MOTORES ---
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
  
  stop_Stop();
  Serial.begin(115200);   
  Serial1.begin(115200);  

  WiFi.init(&Serial1);
  WiFi.beginAP(ssid, 10, pass, ENC_TYPE_WPA2_PSK);
  Udp.begin(localPort);
  Serial.println(F("Sistema Mecanum v4.0 en línea"));
}

void loop() {
  revisarTelemetria();
}

// ========================================================
// ============ PROCESADOR DE COMANDOS UDP ================
// ========================================================
void revisarTelemetria() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    char packetBuffer[256];
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = '\0';
      
      // COMANDO C: Actualizar base de datos geométrica
      if (packetBuffer[0] == 'C') {
        char *token = strtok(packetBuffer, ",");
        if (token != NULL) {
          token = strtok(NULL, ","); if(token) figura_a_dibujar = atoi(token);
          token = strtok(NULL, ","); if(token) ms_cm_frente_triangulo = atof(token);
          token = strtok(NULL, ","); if(token) ms_cm_diagonal         = atof(token);
          token = strtok(NULL, ","); if(token) ms_cm_frente_cuad_rect = atof(token);
          token = strtok(NULL, ","); if(token) ms_cm_lateral          = atof(token);
          
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
          Serial.println(F("Base de variables actualizada."));
        }
      } 
      // COMANDO G: Iniciar ejecución inmediata de la figura seleccionada
      else if (packetBuffer[0] == 'G') {
        if (figura_a_dibujar == 1)      { dibujarTriangulo(); } 
        else if (figura_a_dibujar == 2) { dibujarCuadrado(); } 
        else if (figura_a_dibujar == 3) { dibujarRectangulo(); }
      }
      // COMANDO S: Parada forzosa e inmediata
      else if (packetBuffer[0] == 'S') {
        stop_Stop();
      }
    }
  }
}

// ========================================================
// ============ RUTINAS CINEMÁTICAS HOLONÓMICAS ===========
// ========================================================

void dibujarTriangulo() {
  // LADO 1: Adelante puro
  int t1 = tri_adelante * ms_cm_frente_triangulo;
  setMotors(vel_tri_adelante, vel_tri_adelante, vel_tri_adelante, vel_tri_adelante);
  delay(t1); if (tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  // LADO 2: Diagonal Abajo-Derecha (Giro controlado por angulo_tri_der)
  int t2 = tri_diag_der * ms_cm_diagonal;
  int v2_suave = vel_tri_diag_der * angulo_tri_der;
  setMotors(v2_suave, -vel_tri_diag_der, -vel_tri_diag_der, v2_suave);
  delay(t2); if (tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  // LADO 3: Diagonal Abajo-Izquierda (Giro controlado por angulo_tri_izq)
  int t3 = tri_diag_izq * ms_cm_diagonal;
  int v3_suave = vel_tri_diag_izq * angulo_tri_izq;
  setMotors(-vel_tri_diag_izq, v3_suave, v3_suave, -vel_tri_diag_izq);
  delay(t3); stop_Stop();
}

void dibujarCuadrado() {
  // 1. Adelante
  int t_f = cuad_adelante * ms_cm_frente_cuad_rect;
  setMotors(vel_cuad_adelante, vel_cuad_adelante, vel_cuad_adelante, vel_cuad_adelante);
  delay(t_f); if (tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  // 2. Lateral Derecha (Strafe + Inyección de control de deriva)
  int t_d = cuad_derecha * ms_cm_lateral;
  int v_d = vel_cuad_derecha; int d_d = deriva_cuad_der;
  setMotors(v_d + d_d, -v_d + d_d, -v_d + d_d, v_d + d_d);
  delay(t_d); if (tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  // 3. Atrás
  int t_at = cuad_atras * ms_cm_frente_cuad_rect;
  setMotors(-vel_cuad_atras, -vel_cuad_atras, -vel_cuad_atras, -vel_cuad_atras);
  delay(t_at); if (tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  // 4. Lateral Izquierda (Strafe + Inyección de control de deriva)
  int t_i = cuad_izquierda * ms_cm_lateral;
  int v_i = vel_cuad_izquierda; int d_i = deriva_cuad_izq;
  setMotors(-v_i + d_i, v_i + d_i, v_i + d_i, -v_i + d_i);
  delay(t_i); stop_Stop();
}

void dibujarRectangulo() {
  // 1. Adelante
  int t_f = rect_adelante * ms_cm_frente_cuad_rect;
  setMotors(vel_rect_adelante, vel_rect_adelante, vel_rect_adelante, vel_rect_adelante);
  delay(t_f); if (tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  // 2. Lateral Derecha
  int t_d = rect_derecha * ms_cm_lateral;
  int v_d = vel_rect_derecha; int d_d = deriva_rect_der;
  setMotors(v_d + d_d, -v_d + d_d, -v_d + d_d, v_d + d_d);
  delay(t_d); if (tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  // 3. Atrás
  int t_at = rect_atras * ms_cm_frente_cuad_rect;
  setMotors(-vel_rect_atras, -vel_rect_atras, -vel_rect_atras, -vel_rect_atras);
  delay(t_at); if (tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  // 4. Lateral Izquierda
  int t_i = rect_izquierda * ms_cm_lateral;
  int v_i = vel_rect_izquierda; int d_i = deriva_rect_izq;
  setMotors(-v_i + d_i, v_i + d_i, v_i + d_i, -v_i + d_i);
  delay(t_i); stop_Stop();
}

// ========================================================
// ============ CONTROLADOR DE POLARIDAD MOTOR ============
// ========================================================
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