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
marchas      = [   80,   130,   190,   255  ]
nombres      = ["LENTA", "BASE", "RÁPIDA", "TURBO"]

indice_marcha = 1  # Empezamos en BASE

print(f"Iniciando en >> MODO {nombres[indice_marcha]} ({marchas[indice_marcha]}/255)")

try:
    while True:
        pygame.event.pump()
        
        # --- LECTURA DE LOS JOYSTICKS ---
        x = mando.get_axis(0)  
        y = -mando.get_axis(1) 
        r = mando.get_axis(2)  

        if abs(x) < DEADZONE: x = 0
        if abs(y) < DEADZONE: y = 0
        if abs(r) < DEADZONE: r = 0

        # --- LECTURA DE GATILLOS Y BOTONES TRASEROS ---
        try:
            l2_val = mando.get_axis(4) 
            r2_val = mando.get_axis(5) 
        except:
            l2_val = -1.0
            r2_val = -1.0

        try:
            l1_pulsado = mando.get_button(4) or (mando.get_numbuttons() > 9 and mando.get_button(9))
            r1_pulsado = mando.get_button(5) or (mando.get_numbuttons() > 10 and mando.get_button(10))
        except:
            l1_pulsado = False
            r1_pulsado = False

        # --- LÓGICA DE LA CAJA DE CAMBIOS ---
        nuevo_indice = indice_marcha

        if l1_pulsado:
            nuevo_indice = 0       
        elif l2_val > 0.0:
            nuevo_indice = 1       
        elif r1_pulsado:
            nuevo_indice = 2       
        elif r2_val > 0.0:
            nuevo_indice = 3       

        if nuevo_indice != indice_marcha:
            indice_marcha = nuevo_indice
            print(f">> CAMBIO A: MODO {nombres[indice_marcha]} ({marchas[indice_marcha]}/255)")

        # --- NUEVO: LÓGICA DE LAS PINZAS ---
        try:
            btn_x = mando.get_button(0)       # Botón X (Cruz)
            btn_circulo = mando.get_button(1) # Botón Círculo
        except:
            btn_x = False
            btn_circulo = False

        estado_pinza = 0 # Por defecto: 0 (Quieto)
        if btn_x and not btn_circulo:
            estado_pinza = 1  # 1 = Cerrar
        elif btn_circulo and not btn_x:
            estado_pinza = -1 # -1 = Abrir

        # --- MATEMÁTICAS DE LAS RUEDAS ---
        velocidad_actual = marchas[indice_marcha]

        front_left  = y + x + r
        front_right = y - x - r
        back_left   = y - x + r
        back_right  = y + x - r

        max_val = max(abs(front_left), abs(front_right), abs(back_left), abs(back_right), 1.0)
        
        fl = int((front_left / max_val) * velocidad_actual)
        fr = int((front_right / max_val) * velocidad_actual)
        bl = int((back_left / max_val) * velocidad_actual)
        br = int((back_right / max_val) * velocidad_actual)

        # --- ENVÍO DE DATOS (AHORA SON 5 VALORES) ---
        # Formato: Rueda1, Rueda2, Rueda3, Rueda4, Pinza
        paquete = f"{fl},{fr},{bl},{br},{estado_pinza}\n".encode()
        sock.sendto(paquete, (ROBOT_IP, PORT))
        
        time.sleep(0.02) 

except KeyboardInterrupt:
    print("\nConexión finalizada.")
    pygame.quit()