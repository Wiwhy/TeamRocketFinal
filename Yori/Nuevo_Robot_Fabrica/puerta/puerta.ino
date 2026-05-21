#include "WiFiEsp.h"
#include "WiFiEspUdp.h"

char ssid[] = "ROBOT_COMPETICION"; 
char pass[] = "12345678"; 
int status = WL_IDLE_STATUS;
unsigned int localPort = 8081; 
WiFiEspUDP Udp;

void setup() {
  Serial.begin(115200);   
  Serial1.begin(115200);  
  
  WiFi.init(&Serial1);

  Serial.print(F("Conectando a la red del robot: "));
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
        Serial.println(F("\n[RECIBIDO] -> COMANDO: ABRIR"));
      } 
      else if (strncmp(packetBuffer, "CERRAR", 6) == 0) {
        Serial.println(F("\n[RECIBIDO] -> COMANDO: CERRAR"));
      }
    }
  }
}