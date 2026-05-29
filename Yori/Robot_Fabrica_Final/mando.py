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
marchas      = [   80,   130,   190,   255  ]
nombres      = ["LENTA", "BASE", "RÁPIDA", "TURBO"]
indice_marcha = 1  

print(f"Iniciando en >> MODO {nombres[indice_marcha]}")

estado_abrir_ant = False
estado_cerrar_ant = False

try:
    while True:
        pygame.event.pump()
        
        # --- LECTURA DE LOS JOYSTICKS (MECANUM) ---
        x = mando.get_axis(0)  
        y = -mando.get_axis(1) 
        r = mando.get_axis(2)  

        if abs(x) < DEADZONE: x = 0
        if abs(y) < DEADZONE: y = 0
        if abs(r) < DEADZONE: r = 0

        # --- LÓGICA DE LA PUERTA (CRUCETA) ---
        btn_abrir = False; btn_cerrar = False
        
        try:
            if mando.get_numhats() > 0:
                cruceta = mando.get_hat(0)
                if cruceta[1] == 1: btn_abrir = True    
                elif cruceta[1] == -1: btn_cerrar = True 
                
            if mando.get_numbuttons() > 12:
                if mando.get_button(11): btn_abrir = True
                if mando.get_button(12): btn_cerrar = True
        except:
            pass
            
        if btn_abrir and not estado_abrir_ant:
            sock.sendto(b"PUERTA_ABRIR\n", (ROBOT_IP, PORT))
            estado_abrir_ant = True
            time.sleep(0.1) 
            continue 
            
        if btn_cerrar and not estado_cerrar_ant:
            sock.sendto(b"PUERTA_CERRAR\n", (ROBOT_IP, PORT))
            estado_cerrar_ant = True
            time.sleep(0.1) 
            continue 

        estado_abrir_ant = btn_abrir
        estado_cerrar_ant = btn_cerrar

        # --- LÓGICA DEL SERVO (BOTONES) ---
        try:
            btn_x = mando.get_button(0)       
            btn_circulo = mando.get_button(1) 
        except:
            btn_x = False
            btn_circulo = False

        estado_pinza = 0 
        if btn_x and not btn_circulo: estado_pinza = 1  
        elif btn_circulo and not btn_x: estado_pinza = -1 

        # --- LÓGICA DE CAJA DE CAMBIOS ---
        try:
            l2_val = mando.get_axis(4) 
            r2_val = mando.get_axis(5) 
            l1_pulsado = mando.get_button(4) or (mando.get_numbuttons() > 9 and mando.get_button(9))
            r1_pulsado = mando.get_button(5) or (mando.get_numbuttons() > 10 and mando.get_button(10))
        except:
            l2_val = -1.0; r2_val = -1.0; l1_pulsado = False; r1_pulsado = False

        nuevo_indice = indice_marcha
        if l1_pulsado: nuevo_indice = 0       
        elif l2_val > 0.0: nuevo_indice = 1       
        elif r1_pulsado: nuevo_indice = 2       
        elif r2_val > 0.0: nuevo_indice = 3       

        if nuevo_indice != indice_marcha:
            indice_marcha = nuevo_indice
            print(f">> CAMBIO A: MODO {nombres[indice_marcha]}")

        # --- MATEMÁTICAS MECANUM (4 MOTORES) ---
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

        # Enviar 5 valores: Rueda1, Rueda2, Rueda3, Rueda4, Servo
        paquete = f"{fl},{fr},{bl},{br},{estado_pinza}\n".encode()
        sock.sendto(paquete, (ROBOT_IP, PORT))
        
        time.sleep(0.02) 

except KeyboardInterrupt:
    pygame.quit()