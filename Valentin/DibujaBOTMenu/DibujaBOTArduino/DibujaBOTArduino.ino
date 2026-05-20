#include "WiFiEsp.h"
#include "WiFiEspUdp.h"

char ssid[] = "ROBOT_COMPETICION"; 
char pass[] = "12345678"; 
int status = WL_IDLE_STATUS;
unsigned int localPort = 8080;
WiFiEspUDP Udp;

int figura_a_dibujar = 1;
int tiempo_espera_esquinas = 500;

// Variables de tiempo directo en ms para cada lado por separado
int ms_tri_adelante = 525;
int ms_tri_diag_der = 1225;
int ms_tri_diag_izq = 1225;

int ms_cuad_adelante = 560;
int ms_cuad_derecha  = 866;
int ms_cuad_atras    = 560;
int ms_cuad_izquierda= 866;

int ms_rect_adelante = 784;
int ms_rect_derecha  = 520;
int ms_rect_atras    = 784;
int ms_rect_izquierda= 520;

// Parámetros de velocidades y giros
int vel_tri_adelante = 100;   
int vel_tri_diag_der = 100;   float angulo_tri_der = 0.60;
int vel_tri_diag_izq = 100;   float angulo_tri_izq = 0.60;

int vel_cuad_adelante = 75;   
int vel_cuad_derecha  = 75;   float deriva_cuad_der = 0.0;
int vel_cuad_atras    = 85;   
int vel_cuad_izquierda= 75;   float deriva_cuad_izq = 0.0;

int vel_rect_adelante = 75;   
int vel_rect_derecha  = 75;   float deriva_rect_der = 0.0;
int vel_rect_atras    = 85;   
int vel_rect_izquierda= 75;   float deriva_rect_izq = 0.0;

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
  Serial.println(F("Sistema Tiempos Directos v5.0"));
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
          
          // Triángulo
          token = strtok(NULL, ","); if(token) vel_tri_adelante = atoi(token);
          token = strtok(NULL, ","); if(token) ms_tri_adelante  = atoi(token);
          token = strtok(NULL, ","); if(token) vel_tri_diag_der = atoi(token);
          token = strtok(NULL, ","); if(token) ms_tri_diag_der  = atoi(token);
          token = strtok(NULL, ","); if(token) angulo_tri_der   = atof(token);
          token = strtok(NULL, ","); if(token) vel_tri_diag_izq = atoi(token);
          token = strtok(NULL, ","); if(token) ms_tri_diag_izq  = atoi(token);
          token = strtok(NULL, ","); if(token) angulo_tri_izq   = atof(token);
          
          // Cuadrado
          token = strtok(NULL, ","); if(token) vel_cuad_adelante = atoi(token);
          token = strtok(NULL, ","); if(token) ms_cuad_adelante  = atoi(token);
          token = strtok(NULL, ","); if(token) vel_cuad_derecha  = atoi(token);
          token = strtok(NULL, ","); if(token) ms_cuad_derecha   = atoi(token);
          token = strtok(NULL, ","); if(token) deriva_cuad_der   = atof(token);
          token = strtok(NULL, ","); if(token) vel_cuad_atras    = atoi(token);
          token = strtok(NULL, ","); if(token) ms_cuad_atras      = atoi(token);
          token = strtok(NULL, ","); if(token) vel_cuad_izquierda = atoi(token);
          token = strtok(NULL, ","); if(token) ms_cuad_izquierda = atoi(token);
          token = strtok(NULL, ","); if(token) deriva_cuad_izq   = atof(token);
          
          // Rectángulo
          token = strtok(NULL, ","); if(token) vel_rect_adelante = atoi(token);
          token = strtok(NULL, ","); if(token) ms_rect_adelante  = atoi(token);
          token = strtok(NULL, ","); if(token) vel_rect_derecha  = atoi(token);
          token = strtok(NULL, ","); if(token) ms_rect_derecha   = atoi(token);
          token = strtok(NULL, ","); if(token) deriva_rect_der   = atof(token);
          token = strtok(NULL, ","); if(token) vel_rect_atras    = atoi(token);
          token = strtok(NULL, ","); if(token) ms_rect_atras      = atoi(token);
          token = strtok(NULL, ","); if(token) vel_rect_izquierda = atoi(token);
          token = strtok(NULL, ","); if(token) ms_rect_izquierda = atoi(token);
          token = strtok(NULL, ","); if(token) deriva_rect_izq   = atof(token);
          
          token = strtok(NULL, ","); if(token) tiempo_espera_esquinas = atoi(token);
          
          Serial.println(F("Tiempos de lados actualizados."));
        }
      } 
      else if (packetBuffer[0] == 'G') {
        if (figura_a_dibujar == 1)      { dibujarTriangulo(); } 
        else if (figura_a_dibujar == 2) { dibujarCuadrado(); } 
        else if (figura_a_dibujar == 3) { dibujarRectangulo(); }
      }
      else if (packetBuffer[0] == 'S') {
        stop_Stop();
      }
    }
  }
}

void dibujarTriangulo() {
  setMotors(vel_tri_adelante, vel_tri_adelante, vel_tri_adelante, vel_tri_adelante);
  delay(ms_tri_adelante); 
  if (tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  int v2_suave = vel_tri_diag_der * angulo_tri_der;
  setMotors(v2_suave, -vel_tri_diag_der, -vel_tri_diag_der, v2_suave);
  delay(ms_tri_diag_der); 
  if (tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  int v3_suave = vel_tri_diag_izq * angulo_tri_izq;
  setMotors(-vel_tri_diag_izq, v3_suave, v3_suave, -vel_tri_diag_izq);
  delay(ms_tri_diag_izq); 
  stop_Stop();
}

void dibujarCuadrado() {
  setMotors(vel_cuad_adelante, vel_cuad_adelante, vel_cuad_adelante, vel_cuad_adelante);
  delay(ms_cuad_adelante);
  if (tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  int v_d = vel_cuad_derecha; int d_d = deriva_cuad_der;
  setMotors(v_d + d_d, -v_d + d_d, -v_d + d_d, v_d + d_d);
  delay(ms_cuad_derecha); 
  if (tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  setMotors(-vel_cuad_atras, -vel_cuad_atras, -vel_cuad_atras, -vel_cuad_atras);
  delay(ms_cuad_atras); 
  if (tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  int v_i = vel_cuad_izquierda; int d_i = deriva_cuad_izq;
  setMotors(-v_i + d_i, v_i + d_i, v_i + d_i, -v_i + d_i);
  delay(ms_cuad_izquierda); 
  stop_Stop();
}

void dibujarRectangulo() {
  setMotors(vel_rect_adelante, vel_rect_adelante, vel_rect_adelante, vel_rect_adelante);
  delay(ms_rect_adelante); 
  if (tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  int v_d = vel_rect_derecha; int d_d = deriva_rect_der;
  setMotors(v_d + d_d, -v_d + d_d, -v_d + d_d, v_d + d_d);
  delay(ms_rect_derecha); 
  if (tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  setMotors(-vel_rect_atras, -vel_rect_atras, -vel_rect_atras, -vel_rect_atras);
  delay(ms_rect_atras); 
  if (tiempo_espera_esquinas > 0) { stop_Stop(); delay(tiempo_espera_esquinas); }

  int v_i = vel_rect_izquierda; int d_i = deriva_rect_izq;
  setMotors(-v_i + d_i, v_i + d_i, v_i + d_i, -v_i + d_i);
  delay(ms_rect_izquierda); 
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