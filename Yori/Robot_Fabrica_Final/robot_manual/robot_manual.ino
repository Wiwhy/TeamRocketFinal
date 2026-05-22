#include "WiFiEsp.h"
#include "WiFiEspUdp.h"

char ssid[] = "ROBOT_COMPETICION"; 
char pass[] = "12345678"; 
int status = WL_IDLE_STATUS;
unsigned int localPort = 8080;
WiFiEspUDP Udp;

unsigned int puertoPuerta = 8081;

// ==========================================
// PINES MODO TANQUE (Faja de cables Derecha - M_B)
// ==========================================
// LADO IZQUIERDO (Conectado a M_B IN1, IN2, ENA)
#define MotorIzq_Dir1 22
#define MotorIzq_Dir2 24
#define MotorIzq_PWM  9

// LADO DERECHO (Conectado a M_B IN3, IN4, ENB)
#define MotorDer_Dir1 26
#define MotorDer_Dir2 28
#define MotorDer_PWM  10

// ==========================================
// PINES DE LA PINZA (Faja de cables Izquierda - M_A)
// ==========================================
// PINZA (Conectado a M_A IN1, IN2, ENA)
#define PinzaDir1 5
#define PinzaDir2 6
#define PinzaPWM 11
#define VELOCIDAD_PINZA 150 

int cur_izq = 0, target_izq = 0;
int cur_der = 0, target_der = 0;
int basura1 = 0, basura2 = 0; // Para absorber los dos ceros del Python
int target_pinza = 0;
unsigned long last_millis = 0;

void setup() {
  Serial.begin(115200);   
  Serial1.begin(115200); 

  pinMode(MotorIzq_Dir1, OUTPUT); pinMode(MotorIzq_Dir2, OUTPUT); pinMode(MotorIzq_PWM, OUTPUT);
  pinMode(MotorDer_Dir1, OUTPUT); pinMode(MotorDer_Dir2, OUTPUT); pinMode(MotorDer_PWM, OUTPUT);
  pinMode(PinzaDir1, OUTPUT); pinMode(PinzaDir2, OUTPUT); pinMode(PinzaPWM, OUTPUT);

  apagar_todo();

  WiFi.init(&Serial1);
  WiFi.beginAP(ssid, 10, pass, ENC_TYPE_WPA2_PSK);
  Udp.begin(localPort);
  Serial.println(F("¡Modo Tanque M_B + Pinza M_A + Puerta listo!"));
}

void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    char packetBuffer[64];
    int len = Udp.read(packetBuffer, 63);
    if (len > 0) packetBuffer[len] = '\0';
    
    // ENRUTAMIENTO DE LA PUERTA (SIN ATASCOS)
    if (strncmp(packetBuffer, "PUERTA_ABRIR", 12) == 0) {
      Serial.println(F(">>> Orden ABRIR recibida. Disparando a la red..."));
      disparoAbanico("ABRIR");
    } 
    else if (strncmp(packetBuffer, "PUERTA_CERRAR", 13) == 0) {
      Serial.println(F(">>> Orden CERRAR recibida. Disparando a la red..."));
      disparoAbanico("CERRAR");
    }
    // LECTURA DE TELEMETRÍA (TANQUE + PINZA)
    else {
      sscanf(packetBuffer, "%d,%d,%d,%d,%d", &target_izq, &target_der, &basura1, &basura2, &target_pinza);
    }
  }

  // FÍSICA Y MOVIMIENTO
  if (millis() - last_millis >= 10) { 
    last_millis = millis();
    
    suavizar_rueda(cur_izq, target_izq);
    suavizar_rueda(cur_der, target_der);

    aplicarMotorTanque(1, cur_izq); 
    aplicarMotorTanque(2, cur_der); 
    
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
  if (estado == 1) { digitalWrite(PinzaDir1, HIGH); digitalWrite(PinzaDir2, LOW); analogWrite(PinzaPWM, VELOCIDAD_PINZA); } 
  else if (estado == -1) { digitalWrite(PinzaDir1, LOW); digitalWrite(PinzaDir2, HIGH); analogWrite(PinzaPWM, VELOCIDAD_PINZA); } 
  else { digitalWrite(PinzaDir1, LOW); digitalWrite(PinzaDir2, LOW); analogWrite(PinzaPWM, 0); }
}

void suavizar_rueda(int &current, int target) {
  if (target == 0 || (current > 0 && target < 0) || (current < 0 && target > 0)) { current = target; return; }
  if (abs(target) < abs(current)) { current = target; return; }
  if (current < target) { current += 8; if (current > target) current = target; } 
  else if (current > target) { current -= 8; if (current < target) current = target; }
}

void aplicarMotorTanque(int lado, int velocidad) {
  bool hacia_adelante = (velocidad >= 0);
  int pwm = abs(velocidad);
  if (pwm > 255) pwm = 255;

  if (lado == 1) { // LADO IZQUIERDO
    digitalWrite(MotorIzq_Dir1, hacia_adelante ? LOW : HIGH);
    digitalWrite(MotorIzq_Dir2, hacia_adelante ? HIGH : LOW);
    analogWrite(MotorIzq_PWM, pwm);
  } else { // LADO DERECHO
    digitalWrite(MotorDer_Dir1, hacia_adelante ? LOW : HIGH);
    digitalWrite(MotorDer_Dir2, hacia_adelante ? HIGH : LOW);
    analogWrite(MotorDer_PWM, pwm);
  }
}

void apagar_todo() {
  target_izq = 0; target_der = 0; target_pinza = 0;
  aplicarMotorTanque(1, 0); aplicarMotorTanque(2, 0); aplicarPinza(0);
}