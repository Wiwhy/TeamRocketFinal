#include "WiFiEsp.h"
#include "WiFiEspUdp.h"

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

volatile long ticks_FL = 0;
volatile long ticks_FR = 0;
volatile long ticks_BL = 0;
volatile long ticks_BR = 0;
bool emergencia_activa = false;

// Variables configurables por Wi-Fi
int figura_activa = 1;
int global_pwm = 150;
int tiempo_pausa = 500;
long tk_c_recto = 840, tk_c_lat = 1000;
long tk_r_recto = 1200, tk_r_lat = 600;
long tk_tri = 1000;
float tri_r1 = 26.8, tri_r2 = 26.8;
float lat_fl = 100.0, lat_fr = 100.0, lat_bl = 100.0, lat_br = 100.0;

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

  detenerTodos();
  Serial.begin(115200); Serial1.begin(115200);  
  WiFi.init(&Serial1);
  WiFi.beginAP(ssid, 10, pass, ENC_TYPE_WPA2_PSK);
  Udp.begin(localPort);
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
      
      if (packetBuffer[0] == 'S') {
        emergencia_activa = true;
        detenerTodos();
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
        token = strtok(NULL, ","); if(token) tk_tri = atol(token);
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
// === CONTROL ODOMÉTRICO MATRICIAL DE ALTA PRECISIÓN ===
// ========================================================
void avanzarIndependiente(long tFL, long tFR, long tBL, long tBR, int pFL, int pFR, int pBL, int pBR) {
  ticks_FL = 0; ticks_FR = 0; ticks_BL = 0; ticks_BR = 0;
  bool doneFL = (tFL == 0), doneFR = (tFR == 0), doneBL = (tBL == 0), doneBR = (tBR == 0);

  // El bucle se mantiene vivo hasta que LAS 4 ruedas alcanzan sus objetivos o se pulsa Stop.
  while (!emergencia_activa && (!doneFL || !doneFR || !doneBL || !doneBR)) {
    revisarTelemetria(); 
    if (emergencia_activa) break;

    // Rueda 1: Evalúa, aplica PWM y frena instantáneamente al llegar
    if (!doneFL) {
      if (abs(ticks_FL) >= abs(tFL)) { apagarMotor(0); doneFL = true; }
      else { aplicarPotenciaDriver(0, pFL, (tFL > 0) ? 1 : -1); }
    }
    // Rueda 2
    if (!doneFR) {
      if (abs(ticks_FR) >= abs(tFR)) { apagarMotor(1); doneFR = true; }
      else { aplicarPotenciaDriver(1, pFR, (tFR > 0) ? 1 : -1); }
    }
    // Rueda 3
    if (!doneBL) {
      if (abs(ticks_BL) >= abs(tBL)) { apagarMotor(2); doneBL = true; }
      else { aplicarPotenciaDriver(2, pBL, (tBL > 0) ? 1 : -1); }
    }
    // Rueda 4
    if (!doneBR) {
      if (abs(ticks_BR) >= abs(tBR)) { apagarMotor(3); doneBR = true; }
      else { aplicarPotenciaDriver(3, pBR, (tBR > 0) ? 1 : -1); }
    }
  }
  detenerTodos();
}

void pausaMecanica(int tiempo) {
  unsigned long inicio = millis();
  while (millis() - inicio < tiempo) {
    revisarTelemetria();
    if (emergencia_activa) break;
  }
}

// ========================================================
// ================== FIGURAS GEOMÉTRICAS =================
// ========================================================
void dibujarTriangulo() {
  // Lado 1: Adelante
  avanzarIndependiente(tk_tri, tk_tri, tk_tri, tk_tri, global_pwm, global_pwm, global_pwm, global_pwm);
  if(emergencia_activa) return; pausaMecanica(tiempo_pausa);

  // Lado 2: Diagonal Atrás-Derecha (Giro 1)
  long tk_lenta1 = tk_tri * (tri_r1 / 100.0);
  int pwm_lenta1 = global_pwm * (tri_r1 / 100.0);
  // FL y BR van atrás (Rápidas 100%). FR y BL van adelante (Lentas %).
  avanzarIndependiente(-tk_tri, tk_lenta1, tk_lenta1, -tk_tri, global_pwm, pwm_lenta1, pwm_lenta1, global_pwm);
  if(emergencia_activa) return; pausaMecanica(tiempo_pausa);

  // Lado 3: Diagonal Atrás-Izquierda (Giro 2)
  long tk_lenta2 = tk_tri * (tri_r2 / 100.0);
  int pwm_lenta2 = global_pwm * (tri_r2 / 100.0);
  // FR y BL van atrás (Rápidas 100%). FL y BR van adelante (Lentas %).
  avanzarIndependiente(tk_lenta2, -tk_tri, -tk_tri, tk_lenta2, pwm_lenta2, global_pwm, global_pwm, pwm_lenta2);
}

void dibujarCuadrado() {
  // L1: Adelante
  avanzarIndependiente(tk_c_recto, tk_c_recto, tk_c_recto, tk_c_recto, global_pwm, global_pwm, global_pwm, global_pwm);
  if(emergencia_activa) return; pausaMecanica(tiempo_pausa);

  // L2: Derecha (Aplica la corrección de deriva a las velocidades)
  avanzarIndependiente(tk_c_lat, -tk_c_lat, -tk_c_lat, tk_c_lat,
                       global_pwm * (lat_fl/100.0), global_pwm * (lat_fr/100.0),
                       global_pwm * (lat_bl/100.0), global_pwm * (lat_br/100.0));
  if(emergencia_activa) return; pausaMecanica(tiempo_pausa);

  // L3: Atrás
  avanzarIndependiente(-tk_c_recto, -tk_c_recto, -tk_c_recto, -tk_c_recto, global_pwm, global_pwm, global_pwm, global_pwm);
  if(emergencia_activa) return; pausaMecanica(tiempo_pausa);

  // L4: Izquierda (Aplica la corrección de deriva a las velocidades)
  avanzarIndependiente(-tk_c_lat, tk_c_lat, tk_c_lat, -tk_c_lat,
                       global_pwm * (lat_fl/100.0), global_pwm * (lat_fr/100.0),
                       global_pwm * (lat_bl/100.0), global_pwm * (lat_br/100.0));
}

void dibujarRectangulo() {
  // L1: Adelante (Largo)
  avanzarIndependiente(tk_r_recto, tk_r_recto, tk_r_recto, tk_r_recto, global_pwm, global_pwm, global_pwm, global_pwm);
  if(emergencia_activa) return; pausaMecanica(tiempo_pausa);

  // L2: Derecha (Corto)
  avanzarIndependiente(tk_r_lat, -tk_r_lat, -tk_r_lat, tk_r_lat,
                       global_pwm * (lat_fl/100.0), global_pwm * (lat_fr/100.0),
                       global_pwm * (lat_bl/100.0), global_pwm * (lat_br/100.0));
  if(emergencia_activa) return; pausaMecanica(tiempo_pausa);

  // L3: Atrás (Largo)
  avanzarIndependiente(-tk_r_recto, -tk_r_recto, -tk_r_recto, -tk_r_recto, global_pwm, global_pwm, global_pwm, global_pwm);
  if(emergencia_activa) return; pausaMecanica(tiempo_pausa);

  // L4: Izquierda (Corto)
  avanzarIndependiente(-tk_r_lat, tk_r_lat, tk_r_lat, -tk_r_lat,
                       global_pwm * (lat_fl/100.0), global_pwm * (lat_fr/100.0),
                       global_pwm * (lat_bl/100.0), global_pwm * (lat_br/100.0));
}

// ========================================================
// === CONTROL HARDWARE L298N (Direcciones Reales) ========
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

void apagarMotor(int motorIndex) {
  switch(motorIndex) {
    case 0: analogWrite(speedPinL, 0); break;
    case 1: analogWrite(speedPinR, 0); break;
    case 2: analogWrite(speedPinLB, 0); break;
    case 3: analogWrite(speedPinRB, 0); break;
  }
}

void detenerTodos() {
  apagarMotor(0); apagarMotor(1); apagarMotor(2); apagarMotor(3);
}