#include "WiFiEsp.h"
#include "WiFiEspUdp.h"

char ssid[] = "ROBOT_COMPETICION"; 
char pass[] = "12345678"; 
unsigned int localPort = 8080;
WiFiEspUDP Udp;

// ========================================================
// MEMORIA DUAL DE PERFILES (Índice 0 = BOLO, Índice 1 = SUMO)
// ========================================================
bool MODO_SUMO = true; 
bool en_marcha = false; // El robot arranca PARADO por seguridad

int IN_RETRO[2] = {0, 1}; // 0 = Falso, 1 = Verdadero
int D_DETEC[2]  = {200, 100};
int T_IN_AV[2]  = {1680, 800};
int T_RET[2]    = {1470, 700};
int V_BUSCA[2]  = {120, 180};
int T_GIRO_B[2] = {40, 20};
int P_PRE_M[2]  = {30, 15};
unsigned long US_TIME[2] = {15000UL, 6000UL};

int T_RECTO[2]  = {70, 70};
int T_GIRO[2]   = {50, 50};
int P_LINEA[2]  = {0, 0};
int V_IN_RET[2] = {255, 255};
int V_IN_AV[2]  = {200, 200};
int V_ATAQUE[2] = {255, 255};
int V_RET[2]    = {200, 200};
unsigned long TM_BUSCA[2] = {5000UL, 5000UL};
int D_PRE[2]    = {10, 10};
int D_POST[2]   = {200, 200};

// PINES
#define speedPinR          9
#define RightMotorDirPin1  22
#define RightMotorDirPin2  24
#define LeftMotorDirPin1   26
#define LeftMotorDirPin2   28
#define speedPinL          10
#define speedPinRB         11
#define RightMotorDirPin1B  5
#define RightMotorDirPin2B  6
#define LeftMotorDirPin1B   7
#define LeftMotorDirPin2B   8
#define speedPinLB         12
#define TRIG  30
#define ECHO  31
#define S1  A4
#define S2  A3
#define S3  A2
#define S4  A1
#define S5  A0
#define ST1  A8
#define ST2  A9
#define ST3  A10
#define ST4  A11
#define ST5  A12

enum Estado { BUSCAR, ATACAR, RETROCEDER };
Estado estado = BUSCAR;
unsigned long tRetroIni = 0;       
unsigned long tBuscarIni = 0;

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200); 
  
  pinMode(RightMotorDirPin1, OUTPUT); pinMode(RightMotorDirPin2, OUTPUT);
  pinMode(LeftMotorDirPin1, OUTPUT);  pinMode(LeftMotorDirPin2, OUTPUT);
  pinMode(speedPinR, OUTPUT);         pinMode(speedPinL, OUTPUT);
  pinMode(RightMotorDirPin1B, OUTPUT);pinMode(RightMotorDirPin2B, OUTPUT);
  pinMode(LeftMotorDirPin1B, OUTPUT); pinMode(LeftMotorDirPin2B, OUTPUT);
  pinMode(speedPinRB, OUTPUT);        pinMode(speedPinLB, OUTPUT);

  pinMode(TRIG, OUTPUT); pinMode(ECHO, INPUT); digitalWrite(TRIG, LOW);
  pinMode(S1, INPUT); pinMode(S2, INPUT); pinMode(S3, INPUT); pinMode(S4, INPUT); pinMode(S5, INPUT);
  pinMode(ST1, INPUT); pinMode(ST2, INPUT); pinMode(ST3, INPUT); pinMode(ST4, INPUT); pinMode(ST5, INPUT);

  parar();
  WiFi.init(&Serial1);
  WiFi.beginAP(ssid, 10, pass, ENC_TYPE_WPA2_PSK);
  Udp.begin(localPort);
}

void escucharWifi() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    char packetBuffer[64];
    int len = Udp.read(packetBuffer, 63);
    if (len > 0) {
      packetBuffer[len] = '\0';
      String msg = String(packetBuffer);
      int firstComma = msg.indexOf(',');
      int secondComma = msg.indexOf(',', firstComma + 1);

      // A. ES UN PARÁMETRO DE TABLA -> "CLAVE,MODO,VALOR"
      if (firstComma > 0 && secondComma > 0) {
        String clave = msg.substring(0, firstComma);
        int m_idx = msg.substring(firstComma + 1, secondComma).toInt();
        long valor = msg.substring(secondComma + 1).toInt();

        if (m_idx == 0 || m_idx == 1) {
          if (clave == "T_RECTO") T_RECTO[m_idx] = valor;
          else if (clave == "T_GIRO") T_GIRO[m_idx] = valor;
          else if (clave == "P_LINEA") P_LINEA[m_idx] = valor;
          else if (clave == "V_IN_RET") V_IN_RET[m_idx] = valor;
          else if (clave == "V_IN_AV") V_IN_AV[m_idx] = valor;
          else if (clave == "V_ATAQUE") V_ATAQUE[m_idx] = valor;
          else if (clave == "V_RET") V_RET[m_idx] = valor;
          else if (clave == "TM_BUSCA") TM_BUSCA[m_idx] = valor;
          else if (clave == "D_PRE") D_PRE[m_idx] = valor;
          else if (clave == "D_POST") D_POST[m_idx] = valor;
          else if (clave == "IN_RETRO") IN_RETRO[m_idx] = valor;
          else if (clave == "D_DETEC") D_DETEC[m_idx] = valor;
          else if (clave == "T_IN_AV") T_IN_AV[m_idx] = valor;
          else if (clave == "T_RET") T_RET[m_idx] = valor;
          else if (clave == "V_BUSCA") V_BUSCA[m_idx] = valor;
          else if (clave == "T_GIRO_B") T_GIRO_B[m_idx] = valor;
          else if (clave == "P_PRE_M") P_PRE_M[m_idx] = valor;
          else if (clave == "US_TIME") US_TIME[m_idx] = valor;
        }
      }
      // B. ES UN COMANDO DEL SISTEMA -> "CLAVE,VALOR"
      else if (firstComma > 0) {
        String clave = msg.substring(0, firstComma);
        long valor = msg.substring(firstComma + 1).toInt();

        if (clave == "MODO") {
          MODO_SUMO = (valor == 1);
        }
        else if (clave == "CMD") {
          if (valor == 1) { // INICIO AUTÓNOMO
            en_marcha = true; estado = BUSCAR; tBuscarIni = millis();
            if (IN_RETRO[MODO_SUMO ? 1 : 0] == 1) maniobraInicio();
          }
          else if (valor == 0) { // PARADO GENERAL
            en_marcha = false; parar();
          }
          else if (valor == 2 && !en_marcha) { velocidad(255); avanzar(); } // MANUAL: ADELANTE
          else if (valor == 3 && !en_marcha) { velocidad(255); retroceder(); } // MANUAL: ATRÁS
          else if (valor == 4 && !en_marcha) { velocidad(255); girarInverso(); } // MANUAL: IZQ
          else if (valor == 5 && !en_marcha) { velocidad(255); girar(); } // MANUAL: DER
        }
      }
    }
  }
}

void loop() {
  escucharWifi(); 
  
  if (!en_marcha) return; // Si el robot está apagado por Wi-Fi, corta el bucle aquí.

  int m = MODO_SUMO ? 1 : 0; // Índice de extracción de datos rápidos
  bool linea = hayLinea();
  
  switch (estado) {
    case BUSCAR:
      if (linea) {
        velocidad(255); girarInverso(); delay(T_GIRO[m]); 
        frenoMagnetico(); delay(50); parar(); delay(P_LINEA[m]);
        estado = RETROCEDER; tRetroIni = millis(); break;
      }
      if (millis() - tBuscarIni >= TM_BUSCA[m]) { estado = ATACAR; break; }
      
      velocidad(V_BUSCA[m]); girar(); delay(T_GIRO_B[m]);
      frenoMagnetico(); delay(15); parar(); delay(P_PRE_M[m]);
      
      if (medirMediana(US_TIME[m]) > 0 && medirMediana(US_TIME[m]) <= D_DETEC[m]) {
        delay(D_PRE[m]); estado = ATACAR;
      }
      break;

    case ATACAR:
      if (linea) {
        velocidad(255); retroceder(); delay(T_RECTO[m]); 
        frenoMagnetico(); delay(50); parar(); delay(P_LINEA[m]);
        estado = RETROCEDER; tRetroIni = millis(); break;
      }
      velocidad(V_ATAQUE[m]); avanzar();
      break;

    case RETROCEDER:
      if (millis() - tRetroIni < (unsigned long)T_RET[m]) {
        velocidad(V_RET[m]); retroceder();
      } else {
        velocidad(255); avanzar(); delay(30); frenoMagnetico(); delay(50); parar();
        delay(D_POST[m]); medirMediana(US_TIME[m]);
        estado = BUSCAR; tBuscarIni = millis();  
      }
      break;
  }
}

void maniobraInicio() {
  int m = MODO_SUMO ? 1 : 0;
  velocidad(V_IN_RET[m]); retroceder();
  while (!hayLineaTrasera()) delay(5);
  velocidad(255); avanzar(); delay(15); frenoMagnetico(); delay(15); parar();
  velocidad(V_IN_AV[m]); avanzar(); delay(T_IN_AV[m]); 
  velocidad(255); retroceder(); delay(T_RECTO[m]);
  frenoMagnetico(); delay(50); parar(); delay(200);
}

int medir(unsigned long timeout) {
  digitalWrite(TRIG, LOW); delayMicroseconds(4);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long us = pulseIn(ECHO, HIGH, timeout); 
  if (us == 0) return 999;        
  int cm = (int)(us / 58);
  if (cm < 2) return 999;          
  return cm;
}

int medirMediana(unsigned long timeout) {
  int a = medir(timeout); delayMicroseconds(500);
  int b = medir(timeout); delayMicroseconds(500);
  int c = medir(timeout);
  if (a > b) { int t = a; a = b; b = t; }
  if (b > c) { int t = b; b = c; c = t; }
  if (a > b) { int t = a; a = b; b = t; }
  return b;  
}

bool hayLinea() { return (!digitalRead(S1) || !digitalRead(S2) || !digitalRead(S3) || !digitalRead(S4) || !digitalRead(S5)); }
bool hayLineaTrasera() { return (!digitalRead(ST1) || !digitalRead(ST2) || !digitalRead(ST3) || !digitalRead(ST4) || !digitalRead(ST5)); }
void velocidad(int v) { analogWrite(speedPinL, v); analogWrite(speedPinR, v); analogWrite(speedPinLB, v); analogWrite(speedPinRB, v); }
void avanzar()      { FRf(); FLf(); RRf(); RLf(); }  
void retroceder()   { FRb(); FLb(); RRb(); RLb(); }  
void girar()        { FRb(); FLf(); RRb(); RLf(); }  
void girarInverso() { FRf(); FLb(); RRf(); RLb(); } 
void frenoMagnetico() {
  digitalWrite(RightMotorDirPin1, LOW); digitalWrite(RightMotorDirPin2, LOW); digitalWrite(LeftMotorDirPin1, LOW); digitalWrite(LeftMotorDirPin2, LOW);
  digitalWrite(RightMotorDirPin1B, LOW); digitalWrite(RightMotorDirPin2B, LOW); digitalWrite(LeftMotorDirPin1B, LOW); digitalWrite(LeftMotorDirPin2B, LOW); velocidad(255);
}
void parar() { frenoMagnetico(); velocidad(0); }
void FRf() { digitalWrite(RightMotorDirPin1, LOW); digitalWrite(RightMotorDirPin2, HIGH); }
void FRb() { digitalWrite(RightMotorDirPin1, HIGH); digitalWrite(RightMotorDirPin2, LOW); }
void FLf() { digitalWrite(LeftMotorDirPin1, LOW); digitalWrite(LeftMotorDirPin2, HIGH); }
void FLb() { digitalWrite(LeftMotorDirPin1, HIGH); digitalWrite(LeftMotorDirPin2, LOW); }
void RRf() { digitalWrite(RightMotorDirPin1B, LOW); digitalWrite(RightMotorDirPin2B, HIGH); }
void RRb() { digitalWrite(RightMotorDirPin1B, HIGH); digitalWrite(RightMotorDirPin2B, LOW); }
void RLf() { digitalWrite(LeftMotorDirPin1B, LOW); digitalWrite(LeftMotorDirPin2B, HIGH); }
void RLb() { digitalWrite(LeftMotorDirPin1B, HIGH); digitalWrite(LeftMotorDirPin2B, LOW); }