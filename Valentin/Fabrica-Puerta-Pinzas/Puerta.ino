#include "WiFiEsp.h"
#include "WiFiEspUdp.h"

char ssid[] = "ROBOT_COMPETICION"; 
char pass[] = "12345678"; 
int status = WL_IDLE_STATUS;
unsigned int localPort = 8081; 
WiFiEspUDP Udp;

void setup() {
  Serial.begin(115200);   // Esto es para que TÚ lo leas bien en el ordenador
  Serial1.begin(9600);    // 🚨 CAMBIO AQUÍ: Probamos a hablarle más lento al chip Wi-Fi

  WiFi.init(&Serial1);

  Serial.println(F("\n--- MEGA EXTERNA (PUERTA) INICIADA ---"));
  Serial.print(F("Conectando a la red del robot..."));
  
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    delay(500);
    Serial.print(".");
  }
  
  Serial.println(F("\n¡CONECTADO AL ROBOT!"));
  Udp.begin(localPort);
  Serial.println(F("Escuchando órdenes en el puerto 8081... Muestra la consola:"));
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
        Serial.println(F("[MENSAJE RECIBIDO] -> ABRIR PUERTA"));
        Serial.println(F("====================================="));
        // Aquí iría el código de los relés en el futuro
      } 
      else if (strncmp(packetBuffer, "CERRAR", 6) == 0) {
        Serial.println(F("====================================="));
        Serial.println(F("[MENSAJE RECIBIDO] -> CERRAR PUERTA"));
        Serial.println(F("====================================="));
        // Aquí iría el código de los relés en el futuro
      }
      else {
        Serial.print(F("Mensaje desconocido: "));
        Serial.println(packetBuffer);
      }
    }
  }
}
