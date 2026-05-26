import pygame
import serial
import time
import sys

# ==========================================
# CONFIGURACIÓN SERIAL (ARDUINO)
# ==========================================
PUERTO = 'COM3'  # <-- Cambia esto al puerto de tu Arduino
BAUDIOS = 9600

# ==========================================
# INICIALIZACIÓN DEL MANDO
# ==========================================
pygame.init()
pygame.joystick.init()

if pygame.joystick.get_count() == 0:
    print("¡Conecta el mando de PS5 primero!")
    sys.exit()

mando = pygame.joystick.Joystick(0)
mando.init()
print(f"Mando detectado: {mando.get_name()}")

# ==========================================
# BUCLE DE ESPERA Y LANZAMIENTO
# ==========================================
try:
    arduino = serial.Serial(PUERTO, BAUDIOS, timeout=1)
    print(f"Conexión Serial establecida en {PUERTO}.")
    time.sleep(2)  # Pausa obligatoria de reinicio del Arduino
    
    print("\n==================================================")
    print(" 🟢 SISTEMA LISTO ")
    print(" Presiona la 'X' (Cruz) en tu mando de PS5 para arrancar")
    print("==================================================\n")

    while True:
        # Vaciamos la cola de eventos
        pygame.event.pump() 

        # En Pygame, la "X" (Cruz) del PS5 suele ser el botón 0. 
        # (Si estás en Linux/macOS a veces cambia al 1 o 2)
        if mando.get_button(0):
            print("¡Arrancando robot!")
            arduino.write(b'X')  # Envía la señal al Arduino
            time.sleep(0.5)      # Evita el rebote (mandar múltiples 'X')
            break                # Cierra el bucle tras disparar
            
    arduino.close()
    pygame.quit()

except Exception as e:
    print(f"Error de conexión: {e}")
    pygame.quit()