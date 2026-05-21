#include "WiFiEsp.h"
#include "WiFiEspUdp.h"

char ssid[] = "ROBOT_COMPETICION"; 
char pass[] = "12345678"; 
int status = WL_IDLE_STATUS;
unsigned int localPort = 8081; // Puerto emparejado con el robot
WiFiEspUDP Udp;

// CONFIGURACIÓN DE IP FIJA PARA LA PUERTA
IPAddress local_IP(192, 168, 4, 10); 
IPAddress gateway(192, 168, 4, 1);   // La IP del robot
IPAddress subnet(255, 255, 255, 0);

void setup() {
  Serial.begin(115200);   
  Serial1.begin(115200);  

  Serial.println(F("\n--- MEGA EXTERNA (PUERTA) INICIADA ---"));
  
  WiFi.init(&Serial1);
  
  // Forzamos a la placa a usar la IP fija antes de conectar
  WiFi.config(local_IP); 

  Serial.print(F("Conectando con IP Fija (192.168.4.10) a la red: "));
  Serial.println(ssid);

  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    delay(500);
    Serial.print(".");
  }
  
  Serial.println(F("\n¡CONECTADO AL ROBOT CON ÉXITO!"));
  Udp.begin(localPort);
  Serial.println(F("Escuchando órdenes en el puerto 8081..."));
}

void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    char packetBuffer[128];
    int len = Udp.read(packetBuffer, 127);
    if (len > 0) {
      packetBuffer[len] = '\0';
      
      if (strncmp(packetBuffer, "ABRIR", 5) == 0) {
        Serial.println(F("====================================="));
        Serial.println(F("[DIRECTO] -> COMANDO RECIBIDO: ABRIR"));
        Serial.println(F("====================================="));
      } 
      else if (strncmp(packetBuffer, "CERRAR", 6) == 0) {
        Serial.println(F("====================================="));
        Serial.println(F("[DIRECTO] -> COMANDO RECIBIDO: CERRAR"));
        Serial.println(F("====================================="));
      }
    }
  }
}