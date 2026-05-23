#include "WiFiEsp.h"
#include "WiFiEspUdp.h"

// ========================================================
// === DECLARACIONES ANTICIPADAS
// ========================================================
void revisarTelemetria();
void avanzarSincronizado(long tFL, long tFR, long tBL, long tBR, int pFL, int pFR, int pBL, int pBR);
void pausaMecanica(int tiempo);
void dibujarTriangulo();
void dibujarCuadrado();
void dibujarRectangulo();
void aplicarPotenciaDriver(int motorIndex, int pwm, int direccion);
void apagarMotorActivo(int motorIndex);
void frenarTodos();
void liberarMotores();

// --- CONFIGURACIÓN WI-FI ---
char ssid[] = "ROBOT_COMPETICION"; 
char pass[] = "12345678"; 
unsigned int localPort = 8080;
WiFiEspUDP Udp;

// --- PINES DE ENCODERS (Interrupciones Hardware) ---
#define PIN_ENC_FL_INT 2
#define PIN_ENC_FR_INT 3
#define PIN_ENC_BL_INT 20
#define PIN_ENC_BR_INT 21

// --- PINES DE POTENCIA MOTORES ---
#define speedPinL 10
#define LeftMotorDirPin1 26
#define LeftMotorDirPin2 28
#define speedPinR 9
#define RightMotorDirPin1 22
#define RightMotorDirPin2 24
#define speedPinLB 12
#define LeftMotorDirPin1B 7
#define LeftMotorDirPin2B 8
#define speedPinRB 11
#define RightMotorDirPin1B 5
#define RightMotorDirPin2B 6

// Contadores en memoria volátil
volatile long ticks_FL = 0;
volatile long ticks_FR = 0;
volatile long ticks_BL = 0;
volatile long ticks_BR = 0;
bool emergencia_activa = false;

// Variables configurables por Wi-Fi (Memoria Buffer)
int figura_activa = 1;
int global_pwm = 250;
int tiempo_pausa = 500;
long tk_c_recto = 840, tk_c_lat = 1000;
long tk_r_recto = 1200, tk_r_lat = 600;
// Nuevas variables independientes para el triángulo
long tk_tri_1 = 1000, tk_tri_2 = 1366, tk_tri_3 = 1366; 
float tri_r1 = 26.8, tri_r2 = 26.8;
float lat_fl = 100.0, lat_fr = 100.0, lat_bl = 100.0, lat_br = 100.0;

// Rutinas de interrupción de hardware
void contar_FL() { ticks_FL++; }
void contar_FR() { ticks_FR++; }
void contar_BL() { ticks_BL++; }
void contar_BR() { ticks_BR++; }

void setup() {
  pinMode(speedPinL, OUTPUT); pinMode(LeftMotorDirPin1, OUTPUT); pinMode(LeftMotorDirPin2, OUTPUT);
  pinMode(speedPinR, OUTPUT); pinMode(RightMotorDirPin1, OUTPUT); pinMode(RightMotorDirPin2, OUTPUT);
  pinMode(speedPinLB, OUTPUT); pinMode(LeftMotorDirPin1B, OUTPUT); pinMode(LeftMotorDirPin2B, OUTPUT);
  pinMode(speedPinRB, OUTPUT); pinMode(RightMotorDirPin1B, OUTPUT); pinMode(RightMotorDirPin2B, OUTPUT);

  pinMode(PIN_ENC_FL_INT, INPUT_PULLUP); pinMode(PIN_ENC_FR_INT, INPUT_PULLUP);
  pinMode(PIN_ENC_BL_INT, INPUT_PULLUP); pinMode(PIN_ENC_BR_INT, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_FL_INT), contar_FL, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_FR_INT), contar_FR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_BL_INT), contar_BL, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_BR_INT), contar_BR, CHANGE);

  liberarMotores();
  Serial.begin(115200); Serial1.begin(115200);  
  
  WiFi.init(&Serial1);
  WiFi.beginAP(ssid, 10, pass, ENC_TYPE_WPA2_PSK);
  Udp.begin(localPort);
  
  Serial.println(F("Sistema de Odometría Vectorial Listo."));
}

void loop() {
  revisarTelemetria();
}

void revisarTelemetria() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    char packetBuffer[256];
    int len = Udp.read((char*)packetBuffer, 255);
    
    if (len > 0) {
      packetBuffer[len] = '\0';
      
      if (packetBuffer[0] == 'S') {
        emergencia_activa = true;
        frenarTodos();
        delay(50);
        liberarMotores();
      } 
      else if (packetBuffer[0] == 'C') {
        char *token = strtok(packetBuffer, ",");
        token = strtok(NULL, ","); if(token) figura_activa = atoi(token);
        token = strtok(NULL, ","); if(token) global_pwm = atoi(token);
        token = strtok(NULL, ","); if(token) tiempo_pausa = atoi(token);
        token = strtok(NULL, ","); if(token) tk_c_recto = atol(token);
        token = strtok(NULL, ","); if(token) tk_c_lat = atol(token);
        token = strtok(NULL, ","); if(token) tk_r_recto = atol(token);
        token = strtok(NULL, ","); if(token) tk_r_lat = atol(token);
        // Recepción de los tres lados del triángulo
        token = strtok(NULL, ","); if(token) tk_tri_1 = atol(token);
        token = strtok(NULL, ","); if(token) tk_tri_2 = atol(token);
        token = strtok(NULL, ","); if(token) tk_tri_3 = atol(token);
        token = strtok(NULL, ","); if(token) tri_r1 = atof(token);
        token = strtok(NULL, ","); if(token) tri_r2 = atof(token);
        token = strtok(NULL, ","); if(token) lat_fl = atof(token);
        token = strtok(NULL, ","); if(token) lat_fr = atof(token);
        token = strtok(NULL, ","); if(token) lat_bl = atof(token);
        token = strtok(NULL, ","); if(token) lat_br = atof(token);
      }
      else if (packetBuffer[0] == 'G') {
        emergencia_activa = false;
        if (figura_activa == 1) dibujarTriangulo();
        else if (figura_activa == 2) dibujarCuadrado();
        else if (figura_activa == 3) dibujarRectangulo();
      }
    }
  }
}

// ========================================================
// === CONTROL ODOMÉTRICO SINCRONIZADO (MAESTRO-ESCLAVO) ==
// ========================================================
void avanzarSincronizado(long tFL, long tFR, long tBL, long tBR, int pFL, int pFR, int pBL, int pBR) {
  
  noInterrupts();
  ticks_FL = 0; ticks_FR = 0; ticks_BL = 0; ticks_BR = 0;
  interrupts();
  
  long max_target = max(max(abs(tFL), abs(tFR)), max(abs(tBL), abs(tBR)));
  if (max_target == 0) return;

  int min_pwm = 65; 
  if (tFL != 0) aplicarPotenciaDriver(0, max(pFL, min_pwm), (tFL > 0) ? 1 : -1);
  if (tFR != 0) aplicarPotenciaDriver(1, max(pFR, min_pwm), (tFR > 0) ? 1 : -1);
  if (tBL != 0) aplicarPotenciaDriver(2, max(pBL, min_pwm), (tBL > 0) ? 1 : -1);
  if (tBR != 0) aplicarPotenciaDriver(3, max(pBR, min_pwm), (tBR > 0) ? 1 : -1);

  unsigned long ultimo_chequeo_wifi = millis();

  while (!emergencia_activa) {
    if (millis() - ultimo_chequeo_wifi >= 100) {
        revisarTelemetria(); 
        ultimo_chequeo_wifi = millis();
        if (emergencia_activa) break;
    }

    long curFL, curFR, curBL, curBR;
    noInterrupts();
    curFL = ticks_FL; curFR = ticks_FR; curBL = ticks_BL; curBR = ticks_BR;
    interrupts();

    long pFL_abs = abs(curFL);
    long pFR_abs = abs(curFR);
    long pBL_abs = abs(curBL);
    long pBR_abs = abs(curBR);

    long progreso_actual = max(max(pFL_abs, pFR_abs), max(pBL_abs, pBR_abs));

    if (progreso_actual >= max_target) {
        frenarTodos();
        break;
    }
  }
  
  delay(50);
  liberarMotores();
}

void pausaMecanica(int tiempo) {
  unsigned long inicio = millis();
  unsigned long ultimo_chequeo = inicio;
  
  while (millis() - inicio < (unsigned long)tiempo) {
    if (millis() - ultimo_chequeo >= 100) { 
      revisarTelemetria(); 
      ultimo_chequeo = millis();
    }
    if (emergencia_activa) break;
  }
}

// ========================================================
// ================== FIGURAS GEOMÉTRICAS =================
// ========================================================
void dibujarTriangulo() {
  // Lado 1: Avance frontal. Utiliza tk_tri_1
  avanzarSincronizado(tk_tri_1, tk_tri_1, tk_tri_1, tk_tri_1, global_pwm, global_pwm, global_pwm, global_pwm);
  if(emergencia_activa) return; pausaMecanica(tiempo_pausa);

  // Lado 2: Diagonal derecha. Utiliza tk_tri_2
  long tk_lenta1 = tk_tri_2 * (tri_r1 / 100.0);
  int pwm_lenta1 = global_pwm * (tri_r1 / 100.0);
  avanzarSincronizado(-tk_tri_2, tk_lenta1, tk_lenta1, -tk_tri_2, global_pwm, pwm_lenta1, pwm_lenta1, global_pwm);
  if(emergencia_activa) return; pausaMecanica(tiempo_pausa);

  // Lado 3: Diagonal izquierda. Utiliza tk_tri_3
  long tk_lenta2 = tk_tri_3 * (tri_r2 / 100.0);
  int pwm_lenta2 = global_pwm * (tri_r2 / 100.0);
  avanzarSincronizado(tk_lenta2, -tk_tri_3, -tk_tri_3, tk_lenta2, pwm_lenta2, global_pwm, global_pwm, pwm_lenta2);
}

void dibujarCuadrado() {
  avanzarSincronizado(tk_c_recto, tk_c_recto, tk_c_recto, tk_c_recto, global_pwm, global_pwm, global_pwm, global_pwm);
  if(emergencia_activa) return; pausaMecanica(tiempo_pausa);

  avanzarSincronizado(tk_c_lat, -tk_c_lat, -tk_c_lat, tk_c_lat,
                       global_pwm * (lat_fl/100.0), global_pwm * (lat_fr/100.0),
                       global_pwm * (lat_bl/100.0), global_pwm * (lat_br/100.0));
  if(emergencia_activa) return; pausaMecanica(tiempo_pausa);

  avanzarSincronizado(-tk_c_recto, -tk_c_recto, -tk_c_recto, -tk_c_recto, global_pwm, global_pwm, global_pwm, global_pwm);
  if(emergencia_activa) return; pausaMecanica(tiempo_pausa);

  avanzarSincronizado(-tk_c_lat, tk_c_lat, tk_c_lat, -tk_c_lat,
                       global_pwm * (lat_fl/100.0), global_pwm * (lat_fr/100.0),
                       global_pwm * (lat_bl/100.0), global_pwm * (lat_br/100.0));
}

void dibujarRectangulo() {
  avanzarSincronizado(tk_r_recto, tk_r_recto, tk_r_recto, tk_r_recto, global_pwm, global_pwm, global_pwm, global_pwm);
  if(emergencia_activa) return; pausaMecanica(tiempo_pausa);

  avanzarSincronizado(tk_r_lat, -tk_r_lat, -tk_r_lat, tk_r_lat,
                       global_pwm * (lat_fl/100.0), global_pwm * (lat_fr/100.0),
                       global_pwm * (lat_bl/100.0), global_pwm * (lat_br/100.0));
  if(emergencia_activa) return; pausaMecanica(tiempo_pausa);

  avanzarSincronizado(-tk_r_recto, -tk_r_recto, -tk_r_recto, -tk_r_recto, global_pwm, global_pwm, global_pwm, global_pwm);
  if(emergencia_activa) return; pausaMecanica(tiempo_pausa);

  avanzarSincronizado(-tk_r_lat, tk_r_lat, tk_r_lat, -tk_r_lat,
                       global_pwm * (lat_fl/100.0), global_pwm * (lat_fr/100.0),
                       global_pwm * (lat_bl/100.0), global_pwm * (lat_br/100.0));
}

// ========================================================
// === CONTROL HARDWARE L298N (Dirección y Frenado) =======
// ========================================================
void aplicarPotenciaDriver(int motorIndex, int pwm, int direccion) {
  bool p1 = (direccion == 1) ? LOW : HIGH;
  bool p2 = (direccion == 1) ? HIGH : LOW;
  switch(motorIndex) {
    case 0: digitalWrite(LeftMotorDirPin1, p1); digitalWrite(LeftMotorDirPin2, p2); analogWrite(speedPinL, pwm); break;
    case 1: digitalWrite(RightMotorDirPin1, p1); digitalWrite(RightMotorDirPin2, p2); analogWrite(speedPinR, pwm); break;
    case 2: digitalWrite(LeftMotorDirPin1B, p1); digitalWrite(LeftMotorDirPin2B, p2); analogWrite(speedPinLB, pwm); break;
    case 3: digitalWrite(RightMotorDirPin1B, p1); digitalWrite(RightMotorDirPin2B, p2); analogWrite(speedPinRB, pwm); break;
  }
}

void apagarMotorActivo(int motorIndex) {
  switch(motorIndex) {
    case 0: digitalWrite(LeftMotorDirPin1, HIGH);  digitalWrite(LeftMotorDirPin2, HIGH);  analogWrite(speedPinL, 255);  break;
    case 1: digitalWrite(RightMotorDirPin1, HIGH); digitalWrite(RightMotorDirPin2, HIGH); analogWrite(speedPinR, 255);  break;
    case 2: digitalWrite(LeftMotorDirPin1B, HIGH);  digitalWrite(LeftMotorDirPin2B, HIGH);  analogWrite(speedPinLB, 255); break;
    case 3: digitalWrite(RightMotorDirPin1B, HIGH); digitalWrite(RightMotorDirPin2B, HIGH); analogWrite(speedPinRB, 255); break;
  }
}

void frenarTodos() {
  apagarMotorActivo(0); apagarMotorActivo(1); apagarMotorActivo(2); apagarMotorActivo(3);
}

void liberarMotores() {
  analogWrite(speedPinL, 0); analogWrite(speedPinR, 0);
  analogWrite(speedPinLB, 0); analogWrite(speedPinRB, 0);
}