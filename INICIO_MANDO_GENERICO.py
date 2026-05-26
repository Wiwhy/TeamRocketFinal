import pygame
import serial
import time
import sys

PUERTO = 'COM3'
BAUDIOS = 9600

pygame.init()
pygame.joystick.init()

if pygame.joystick.get_count() == 0:
    print("Conecta el mando de PS5 primero.")
    sys.exit()

mando = pygame.joystick.Joystick(0)
mando.init()
print(f"Mando detectado: {mando.get_name()}")

try:
    arduino = serial.Serial(PUERTO, BAUDIOS, timeout=1)
    time.sleep(2) 
    
    print("\n[ SISTEMA BLOQUEADO ]")
    print("Presiona la 'X' en el mando para iniciar el combate...")

    while True:
        pygame.event.pump() 
        
        # El botón 0 suele ser la X en mandos de PlayStation en Pygame
        if mando.get_button(0):
            print(">>> ¡ROBOT DESBLOQUEADO Y ARRANCANDO!")
            arduino.write(b'X')
            time.sleep(0.5) 
            break 
            
    arduino.close()
    pygame.quit()

except Exception as e:
    print(f"Error en el puerto serie: {e}")
    pygame.quit()
