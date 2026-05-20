/*
 * ==================================================================================
 * MEGA EXTERNA (SSA) - CONTROLADOR DE LA BARRERA
 * ==================================================================================
 * Se conecta al robot. Escucha en el puerto 8081.
 * Activa los relés y espera a que los sensores físicos (o tu dedo) confirmen la acción.
 * ==================================================================================
 */

#include "WiFiEsp.h"
#include "WiFiEspUdp.h"

// --- CONFIGURACIÓN WI-FI (Se conecta al Robot) ---
char ssid[] = "ROBOT_COMPETICION"; 
char pass[] = "12345678"; 
int status = WL_IDLE_STATUS;
unsigned int localPort = 8081; 
WiFiEspUDP Udp;

// --- PINES DE LOS RELÉS Y SENSORES ---
#define PIN_RELE_ABRIR  22  
#define PIN_RELE_CERRAR 24  
#define PIN_FC_ABIERTO  2   // Pin para simular tope arriba
#define PIN_FC_CERRADO  3   // Pin para simular tope abajo

#define RELE_ENCENDIDO LOW
#define RELE_APAGADO   HIGH

void setup() {
  Serial.begin(115200);   
  Serial1.begin(115200);  

  pinMode(PIN_RELE_ABRIR, OUTPUT);
  pinMode(PIN_RELE_CERRAR, OUTPUT);
  digitalWrite(PIN_RELE_ABRIR, RELE_APAGADO);  
  digitalWrite(PIN_RELE_CERRAR, RELE_APAGADO);

  // Configuramos sensores. Leerán HIGH por defecto.
  pinMode(PIN_FC_ABIERTO, INPUT_PULLUP);
  pinMode(PIN_FC_CERRADO, INPUT_PULLUP);

  Serial.println(F("\n--- SISTEMA DE PUERTA INICIADO ---"));
  Serial.print(F("Buscando red del robot: "));
  Serial.println(ssid);
  
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    delay(500);
    Serial.print(".");
  }
  
  Serial.println(F("\n¡Conectado! SSA de la puerta listo en el puerto 8081."));
}

void loop() {
  int packetSize = Udp.parsePacket();
  
  if (packetSize) {
    char packetBuffer[128];
    int len = Udp.read(packetBuffer, 127);
    
    if (len > 0) {
      packetBuffer[len] = '\0'; 
      
      // ==========================================
      // CASO A: SOLICITUD DE ABRIR
      // ==========================================
      if (strncmp(packetBuffer, "ABRIR", 5) == 0) {
        Serial.println(F("\n[!] ORDEN RECIBIDA: ABRIR LA BARRERA"));
        digitalWrite(PIN_RELE_ABRIR, RELE_ENCENDIDO);
        Serial.println(F(" -> Relé 1 ENCENDIDO (Motor subiendo)."));
        Serial.println(F(" -> [SIMULADOR]: Toca el Pin 2 con GND para simular que la barrera llegó arriba."));

        // Bloqueo: Espera hasta que simules el sensor (Pin 2 toca GND)
        while (digitalRead(PIN_FC_ABIERTO) == HIGH) {
          delay(10); 
        }

        digitalWrite(PIN_RELE_ABRIR, RELE_APAGADO);
        Serial.println(F(" -> [ÉXITO] Sensor de tope presionado. Relé 1 APAGADO. La barrera está abierta."));
      } 
      
      // ==========================================
      // CASO B: SOLICITUD DE CERRAR
      // ==========================================
      else if (strncmp(packetBuffer, "CERRAR", 6) == 0) {
        Serial.println(F("\n[!] ORDEN RECIBIDA: CERRAR LA BARRERA"));
        digitalWrite(PIN_RELE_CERRAR, RELE_ENCENDIDO);
        Serial.println(F(" -> Relé 2 ENCENDIDO (Motor bajando)."));
        Serial.println(F(" -> [SIMULADOR]: Toca el Pin 3 con GND para simular que la barrera llegó al suelo."));

        // Bloqueo: Espera hasta que simules el sensor (Pin 3 toca GND)
        while (digitalRead(PIN_FC_CERRADO) == HIGH) {
          delay(10); 
        }

        digitalWrite(PIN_RELE_CERRAR, RELE_APAGADO);
        Serial.println(F(" -> [ÉXITO] Sensor de suelo presionado. Relé 2 APAGADO. La barrera está cerrada."));
      }
    }
  }
}