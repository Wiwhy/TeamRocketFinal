import pygame
import socket
import time
import sys

ROBOT_IP = "192.168.4.1" 
PORT = 8080
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

pygame.init()
pygame.joystick.init()

if pygame.joystick.get_count() == 0:
    print("¡Conecta el mando de PS5 primero!")
    sys.exit()

mando = pygame.joystick.Joystick(0)
mando.init()
print(f"Mando detectado: {mando.get_name()}")

print("\n==================================================")
print(" 🔴 SISTEMA BLOQUEADO (MODO ESPERA)")
print(f" Red destino: {ROBOT_IP}:{PORT} (UDP)")
print(" Presiona la 'X' (Cruz) en el mando para arrancar")
print("==================================================\n")

try:
    while True:
        pygame.event.pump()
        
        # El botón 0 suele ser la X en PS5 (Pygame)
        if mando.get_button(0):
            # Enviamos el byte 'X' al Arduino
            sock.sendto(b"X", (ROBOT_IP, PORT))
            print("🟢 ¡COMANDO ENVIADO! El robot debería estar moviéndose.")
            time.sleep(1) # Evita mandar 50 paquetes de golpe si dejas pulsado
            break
            
    pygame.quit()

except KeyboardInterrupt:
    print("\nLanzamiento cancelado.")
    pygame.quit()
