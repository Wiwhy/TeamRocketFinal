import pygame
import socket
import math
import threading

# ==========================================
# 1. CONFIGURACIÓN DE RED UDP (Puerto 9000)
# ==========================================
UDP_IP = "0.0.0.0"
UDP_PORT = 9000
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

distancia_actual = 999

def escuchar_udp():
    global distancia_actual
    while True:
        try:
            data, addr = sock.recvfrom(1024)
            distancia_actual = int(data.decode('utf-8').strip())
        except:
            pass

# Hilo en segundo plano para no congelar los gráficos
hilo = threading.Thread(target=escuchar_udp, daemon=True)
hilo.start()

# ==========================================
# 2. CONFIGURACIÓN DE PYGAME (Gráficos)
# ==========================================
pygame.init()
WIDTH, HEIGHT = 800, 600
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Radar Ultrasónico del Robot")

# Paleta de colores
NEGRO = (0, 0, 0)
VERDE_RADAR = (0, 255, 0)
VERDE_OSCURO = (0, 80, 0)
ROJO = (255, 50, 50)

# Geometría del radar
CENTER = (350, 300)
MAX_RADIUS = 250
MAX_DIST_CM = 200 # Límite del radar (2 metros)

angle = 0
blips = [] # Guardará los puntos detectados: [angulo, distancia, opacidad]

font_grande = pygame.font.SysFont("Arial", 48, bold=True)
font_pequena = pygame.font.SysFont("Arial", 16)

clock = pygame.time.Clock()
running = True

# ==========================================
# 3. BUCLE PRINCIPAL (Draw)
# ==========================================
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    # Efecto de estela (Fade) rellenando con negro transparente
    fade_surface = pygame.Surface((WIDTH, HEIGHT))
    fade_surface.set_alpha(15) 
    fade_surface.fill(NEGRO)
    screen.blit(fade_surface, (0, 0))

    # Limpiar el panel lateral derecho al 100% de negro para el texto
    pygame.draw.rect(screen, NEGRO, (650, 0, 150, HEIGHT))

    # A. DIBUJAR LOS ANILLOS DEL RADAR
    for r in range(50, MAX_RADIUS + 1, 50):
        pygame.draw.circle(screen, VERDE_OSCURO, CENTER, r, 1)
        # Etiquetas de distancia en los anillos
        cm_val = int((r / MAX_RADIUS) * MAX_DIST_CM)
        etiqueta = font_pequena.render(f"{cm_val}cm", True, VERDE_OSCURO)
        screen.blit(etiqueta, (CENTER[0] + 5, CENTER[1] - r - 20))

    # B. DIBUJAR LA LÍNEA DE BARRIDO
    x_line = CENTER[0] + MAX_RADIUS * math.cos(math.radians(angle))
    y_line = CENTER[1] + MAX_RADIUS * math.sin(math.radians(angle))
    pygame.draw.line(screen, VERDE_RADAR, CENTER, (x_line, y_line), 3)

    # C. PROCESAR NUEVOS DATOS DEL SENSOR
    if distancia_actual < 999 and distancia_actual <= MAX_DIST_CM:
        # Añadimos el punto con opacidad al máximo (255)
        blips.append([angle, distancia_actual, 255])
        distancia_actual = 999 # Reseteamos para no dibujar el mismo dato 60 veces por segundo

    # D. DIBUJAR LOS PUNTOS (BLIPS) DETECTADOS
    for b in blips:
        b_angle, b_dist, b_alpha = b
        
        # Mapear los centímetros a los píxeles de la pantalla
        radio_pixel = (b_dist / MAX_DIST_CM) * MAX_RADIUS
        bx = CENTER[0] + radio_pixel * math.cos(math.radians(b_angle))
        by = CENTER[1] + radio_pixel * math.sin(math.radians(b_angle))
        
        # Color del punto (Rojo si está a menos de 50cm, Verde si está lejos)
        color_blip = (int(b_alpha), 0, 0) if b_dist <= 50 else (0, int(b_alpha), 0)
        
        pygame.draw.circle(screen, color_blip, (int(bx), int(by)), 6)
        
        # Reducir opacidad poco a poco para que desaparezcan
        b[2] -= 3 
        
    # Limpiar de la memoria los blips que ya son invisibles
    blips = [b for b in blips if b[2] > 0]

    # E. INTERFAZ LATERAL (HUD EN CENTÍMETROS)
    titulo_hud = font_pequena.render("DETECCIÓN:", True, VERDE_RADAR)
    screen.blit(titulo_hud, (660, 40))

    # Mostrar la distancia del último blip válido
    if blips:
        ultima_dist = blips[-1][1]
        texto_dist = f"{ultima_dist} cm"
        color_texto = ROJO if ultima_dist <= 50 else VERDE_RADAR
    else:
        texto_dist = "-- cm"
        color_texto = VERDE_OSCURO

    txt_surface = font_grande.render(texto_dist, True, color_texto)
    screen.blit(txt_surface, (660, 70))

    # F. GIRAR EL RADAR
    angle = (angle + 3) % 360 # Velocidad de rotación

    pygame.display.flip()
    clock.tick(60) # 60 FPS

pygame.quit()