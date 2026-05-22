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
marchas       = [   80,   130,   200,   255  ]
nombres       = ["LENTA", "BASE", "RÁPIDA", "TURBO"]
indice_marcha = 1  

print(f"Iniciando en >> MODO {nombres[indice_marcha]} ({marchas[indice_marcha]}/255)")

estado_abrir_ant = False
estado_cerrar_ant = False

try:
    while True:
        pygame.event.pump()
        
        # --- 1. LECTURA DE EJES ---
        x = mando.get_axis(0)    
        y = -mando.get_axis(1)   
        r = mando.get_axis(2)    
        
        if abs(x) < DEADZONE: x = 0
        if abs(y) < DEADZONE: y = 0
        if abs(r) < DEADZONE: r = 0

        # --- CAJA DE CAMBIOS ---
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

        # --- LÓGICA DE LA PUERTA (BLINDADA CONTRA ATASCOS) ---
        btn_abrir = False; btn_cerrar = False
        teclas = pygame.key.get_pressed()
        
        # Respaldo 100% fiable: Flechas del Teclado de tu PC
        if teclas[pygame.K_UP]: btn_abrir = True
        if teclas[pygame.K_DOWN]: btn_cerrar = True
        
        try:
            # Cruceta PS5 clásica
            if mando.get_numhats() > 0:
                cruceta = mando.get_hat(0)
                if cruceta[1] == 1: btn_abrir = True    
                elif cruceta[1] == -1: btn_cerrar = True 
                
            # Cruceta PS5 Mapeada como botones (11 y 12 suelen ser Arriba/Abajo)
            if mando.get_numbuttons() > 12:
                if mando.get_button(11): btn_abrir = True
                if mando.get_button(12): btn_cerrar = True
        except:
            pass
            
        if btn_abrir and not estado_abrir_ant:
            sock.sendto(b"PUERTA_ABRIR\n", (ROBOT_IP, PORT))
            print("📡 COMANDO ENVIADO: ¡ABRIR barrera!")
            estado_abrir_ant = True
            time.sleep(0.1) # MAGIA: Pausamos Python 0.1s para que el paquete viaje limpio
            continue # Saltamos la telemetría para que no lo atropelle
            
        if btn_cerrar and not estado_cerrar_ant:
            sock.sendto(b"PUERTA_CERRAR\n", (ROBOT_IP, PORT))
            print("📡 COMANDO ENVIADO: ¡CERRAR barrera!")
            estado_cerrar_ant = True
            time.sleep(0.1) 
            continue 

        estado_abrir_ant = btn_abrir
        estado_cerrar_ant = btn_cerrar

        # --- LÓGICA DE PINZA ---
        try:
            btn_x = mando.get_button(0)       
            btn_circulo = mando.get_button(1) 
        except:
            btn_x = False; btn_circulo = False

        estado_pinza = 0 
        if btn_x and not btn_circulo: estado_pinza = 1  
        elif btn_circulo and not btn_x: estado_pinza = -1 

        # --- 2. CINEMÁTICA MECANUM ---
        velocidad_actual = marchas[indice_marcha]
        
        fl = y + x + r
        fr = y - x - r
        bl = y - x + r
        br = y + x - r
        
        max_val = max(abs(fl), abs(fr), abs(bl), abs(br), 1.0)
        
        motor_fl = int((fl / max_val) * velocidad_actual)
        motor_fr = int((fr / max_val) * velocidad_actual)
        motor_bl = int((bl / max_val) * velocidad_actual)
        motor_br = int((br / max_val) * velocidad_actual)

        paquete = f"{motor_fl},{motor_fr},{motor_bl},{motor_br},{estado_pinza}\n".encode()
        sock.sendto(paquete, (ROBOT_IP, PORT))
        time.sleep(0.02) 

except KeyboardInterrupt:
    print("\nConexión finalizada.")
    pygame.quit()