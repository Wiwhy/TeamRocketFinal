import pygame
import socket
import time

ROBOT_IP = "192.168.4.1" 
PORT = 8080
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

pygame.init()
pygame.joystick.init()

if pygame.joystick.get_count() == 0:
    print("¡Conecta el mando de PS5 primero!")
    exit()

mando = pygame.joystick.Joystick(0)
mando.init()
print(f"Mando: {mando.get_name()} | Conexión establecida.")

DEADZONE = 0.2 

# ==========================================
# LA CAJA DE CAMBIOS: SELECCIÓN DIRECTA
# ==========================================
#              [ LENTA, BASE, RÁPIDA, TURBO ]
marchas      = [   80,   130,   190,   255  ]
nombres      = ["LENTA", "BASE", "RÁPIDA", "TURBO"]

indice_marcha = 1  # Empezamos en la posición 1 (BASE = 130) por defecto

print(f"Iniciando en >> MODO {nombres[indice_marcha]} ({marchas[indice_marcha]}/255)")

try:
    while True:
        # Vaciamos la cola de eventos para que no se atasque
        pygame.event.pump()
        
        # --- LECTURA DE LOS JOYSTICKS ---
        x = mando.get_axis(0)  # Joystick Izquierdo (Eje Horizontal)
        y = -mando.get_axis(1) # Joystick Izquierdo (Eje Vertical)
        r = mando.get_axis(2)  # Joystick Derecho (Rotación)

        # Punto muerto
        if abs(x) < DEADZONE: x = 0
        if abs(y) < DEADZONE: y = 0
        if abs(r) < DEADZONE: r = 0

        # --- LECTURA DE BOTONES Y GATILLOS TRASEROS ---
        try:
            # L2 y R2 son ejes (van de -1.0 a 1.0)
            l2_val = mando.get_axis(4) 
            r2_val = mando.get_axis(5) 
        except:
            l2_val = -1.0
            r2_val = -1.0

        try:
            # L1 y R1 son botones (0 o 1). Comprobamos los índices más comunes (4/5 o 9/10)
            l1_pulsado = mando.get_button(4) or (mando.get_numbuttons() > 9 and mando.get_button(9))
            r1_pulsado = mando.get_button(5) or (mando.get_numbuttons() > 10 and mando.get_button(10))
        except:
            l1_pulsado = False
            r1_pulsado = False

        # --- LÓGICA DE LA CAJA DE CAMBIOS ---
        nuevo_indice = indice_marcha

        if l1_pulsado:
            nuevo_indice = 0       # L1 -> LENTA
        elif l2_val > 0.0:
            nuevo_indice = 1       # L2 -> BASE (El eje pasa de 0.0 al apretar a la mitad)
        elif r1_pulsado:
            nuevo_indice = 2       # R1 -> RÁPIDA
        elif r2_val > 0.0:
            nuevo_indice = 3       # R2 -> TURBO

        # Si hemos pulsado un botón distinto, cambiamos la marcha e informamos
        if nuevo_indice != indice_marcha:
            indice_marcha = nuevo_indice
            print(f">> CAMBIO A: MODO {nombres[indice_marcha]} ({marchas[indice_marcha]}/255)")

        # Extraemos la velocidad real
        velocidad_actual = marchas[indice_marcha]

        # Potencia ruedas individuales (Matemáticas Mecanum)
        front_left  = y + x + r
        front_right = y - x - r
        back_left   = y - x + r
        back_right  = y + x - r

        # Evitar que las matemáticas se rompan
        max_val = max(abs(front_left), abs(front_right), abs(back_left), abs(back_right), 1.0)
        
        # Aplicamos la marcha en la que estemos
        fl = int((front_left / max_val) * velocidad_actual)
        fr = int((front_right / max_val) * velocidad_actual)
        bl = int((back_left / max_val) * velocidad_actual)
        br = int((back_right / max_val) * velocidad_actual)

        # Enviar paquete por Wi-Fi
        paquete = f"{fl},{fr},{bl},{br}\n".encode()
        sock.sendto(paquete, (ROBOT_IP, PORT))
        
        time.sleep(0.02) 

except KeyboardInterrupt:
    print("\nConexión finalizada.")
    pygame.quit()