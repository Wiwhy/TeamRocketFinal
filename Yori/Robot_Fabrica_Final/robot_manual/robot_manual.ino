#include "WiFiEsp.h"
#include "WiFiEspUdp.h"
#include <Servo.h>

char ssid[] = "ROBOT_COMPETICION"; 
char pass[] = "12345678"; 
int status = WL_IDLE_STATUS;
unsigned int localPort = 8080;
WiFiEspUDP Udp;

unsigned int puertoPuerta = 8081;

// ==========================================
// PINES DE LAS 4 RUEDAS (MECANUM)
// ==========================================
#define speedPinR 9
#define RightMotorDirPin1  22
#define RightMotorDirPin2  24

#define LeftMotorDirPin1  26
#define LeftMotorDirPin2  28
#define speedPinL 10

#define speedPinRB 11
#define RightMotorDirPin1B  5
#define RightMotorDirPin2B 6

#define LeftMotorDirPin1B 7
#define LeftMotorDirPin2B 8
#define speedPinLB 12

// ==========================================
// CONFIGURACIÓN DEL SERVO (GANCHO)
// ==========================================
#define PIN_SERVO 4
Servo servoGancho;
int angulo_gancho = 90;

// ==========================================
// VARIABLES DE DATOS
// ==========================================
int cur_fl = 0, target_fl = 0;
int cur_fr = 0, target_fr = 0;
int cur_bl = 0, target_bl = 0;
int cur_br = 0, target_br = 0;

int target_pinza = 0; 
unsigned long last_millis = 0;

void setup() {
  Serial.begin(115200);   
  Serial1.begin(115200); 

  pinMode(RightMotorDirPin1, OUTPUT); pinMode(RightMotorDirPin2, OUTPUT); pinMode(speedPinL, OUTPUT);
  pinMode(LeftMotorDirPin1, OUTPUT); pinMode(LeftMotorDirPin2, OUTPUT); pinMode(speedPinR, OUTPUT);
  pinMode(RightMotorDirPin1B, OUTPUT); pinMode(RightMotorDirPin2B, OUTPUT); pinMode(speedPinLB, OUTPUT);
  pinMode(LeftMotorDirPin1B, OUTPUT); pinMode(LeftMotorDirPin2B, OUTPUT); pinMode(speedPinRB, OUTPUT);

  apagar_motores();

  servoGancho.attach(PIN_SERVO);
  servoGancho.write(angulo_gancho);

  WiFi.init(&Serial1);
  WiFi.beginAP(ssid, 10, pass, ENC_TYPE_WPA2_PSK);
  Udp.begin(localPort);
  Serial.println("¡Mecanum + Servo + Puerta listos!");
}

void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    char packetBuffer[64];
    int len = Udp.read(packetBuffer, 63);
    if (len > 0) packetBuffer[len] = '\0';
    
    // FILTRO DE LA PUERTA
    if (strncmp(packetBuffer, "PUERTA_ABRIR", 12) == 0) {
      disparoAbanico("ABRIR");
    } 
    else if (strncmp(packetBuffer, "PUERTA_CERRAR", 13) == 0) {
      disparoAbanico("CERRAR");
    }
    // LECTURA DE TELEMETRÍA (MECANUM + SERVO)
    else {
      sscanf(packetBuffer, "%d,%d,%d,%d,%d", &target_fl, &target_fr, &target_bl, &target_br, &target_pinza);
    }
  }

  // BUCLE FÍSICO (10 ms)
  if (millis() - last_millis >= 10) { 
    last_millis = millis();
    
    suavizar_rueda(cur_fl, target_fl);
    suavizar_rueda(cur_fr, target_fr);
    suavizar_rueda(cur_bl, target_bl);
    suavizar_rueda(cur_br, target_br);

    aplicarMotor(1, cur_fl); 
    aplicarMotor(2, cur_fr); 
    aplicarMotor(3, cur_bl); 
    aplicarMotor(4, cur_br); 
    
    aplicarPinza(target_pinza);
  }
}

// =========================================================
void disparoAbanico(const char* mensaje) {
  for (byte i = 2; i <= 5; i++) {
    IPAddress ipDestino(192, 168, 4, i);
    Udp.beginPacket(ipDestino, puertoPuerta);
    Udp.write(mensaje);
    Udp.endPacket();
    delay(20); 
  }
}

// =========================================================
void aplicarPinza(int estado) {
  if (estado == 1) { 
    angulo_gancho += 3; 
    if (angulo_gancho > 180) angulo_gancho = 180;
  } 
  else if (estado == -1) { 
    angulo_gancho -= 3; 
    if (angulo_gancho < 0) angulo_gancho = 0;
  }
  servoGancho.write(angulo_gancho);
}

void suavizar_rueda(int &current, int target) {
  if (target == 0 || (current > 0 && target < 0) || (current < 0 && target > 0)) { current = target; return; }
  if (abs(target) < abs(current)) { current = target; return; }
  
  if (current < target) {
    current += 8; if (current > target) current = target;
  } else if (current > target) {
    current -= 8; if (current < target) current = target;
  }
}

void aplicarMotor(int motor, int velocidad) {
  bool hacia_adelante = (velocidad >= 0);
  int pwm = abs(velocidad);
  if (pwm > 255) pwm = 255;

  switch(motor) {
    case 1: 
      digitalWrite(LeftMotorDirPin1, hacia_adelante ? LOW : HIGH);
      digitalWrite(LeftMotorDirPin2, hacia_adelante ? HIGH : LOW);
      analogWrite(speedPinL, pwm);
      break;
    case 2: 
      digitalWrite(RightMotorDirPin1, hacia_adelante ? LOW : HIGH);
      digitalWrite(RightMotorDirPin2, hacia_adelante ? HIGH : LOW);
      analogWrite(speedPinR, pwm);
      break;
    case 3: 
      digitalWrite(LeftMotorDirPin1B, hacia_adelante ? LOW : HIGH);
      digitalWrite(LeftMotorDirPin2B, hacia_adelante ? HIGH : LOW);
      analogWrite(speedPinLB, pwm);
      break;
    case 4: 
      digitalWrite(RightMotorDirPin1B, hacia_adelante ? LOW : HIGH);
      digitalWrite(RightMotorDirPin2B, hacia_adelante ? HIGH : LOW);
      analogWrite(speedPinRB, pwm);
      break;
  }
}

void apagar_motores() {
  target_fl = 0; target_fr = 0; target_bl = 0; target_br = 0;
  aplicarMotor(1, 0); aplicarMotor(2, 0); aplicarMotor(3, 0); aplicarMotor(4, 0);
}