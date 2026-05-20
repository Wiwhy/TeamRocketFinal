import tkinter as tk
from tkinter import messagebox
import socket

# --- CONFIGURACIÓN WI-FI ---
ROBOT_IP = "192.168.4.1" 
PORT = 8080
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def enviar_configuracion():
    try:
        # 1. General
        figura = int(var_figura.get())
        t_esq = int(entry_t_esquinas.get())
        
        # 2. Tiempos de Calibración Física
        ms_f_tri = float(entry_ms_f_tri.get())
        ms_diag = float(entry_ms_diag.get())
        ms_f_cr = float(entry_ms_f_cr.get())
        ms_lat = float(entry_ms_lat.get())
        
        # 3. Parámetros del Triángulo
        v_t_ad = int(entry_v_t_ad.get());  t_ad = float(entry_t_ad.get())
        v_t_dd = int(entry_v_t_dd.get());  t_dd = float(entry_t_dd.get()); a_t_d = float(entry_a_t_d.get())
        v_t_di = int(entry_v_t_di.get());  t_di = float(entry_t_di.get()); a_t_i = float(entry_a_t_i.get())
        
        # 4. Parámetros del Cuadrado
        v_c_ad = int(entry_v_c_ad.get());  c_ad = float(entry_c_ad.get())
        v_c_de = int(entry_v_c_de.get());  c_de = float(entry_c_de.get()); d_c_de = float(entry_d_c_de.get())
        v_c_at = int(entry_v_c_at.get());  c_at = float(entry_c_at.get())
        v_c_iz = int(entry_v_c_iz.get());  c_iz = float(entry_c_iz.get()); d_c_iz = float(entry_d_c_iz.get())
        
        # 5. Parámetros del Rectángulo
        v_r_ad = int(entry_v_r_ad.get());  r_ad = float(entry_r_ad.get())
        v_r_de = int(entry_v_r_de.get());  r_de = float(entry_r_de.get()); d_r_de = float(entry_d_r_de.get())
        v_r_at = int(entry_v_r_at.get());  r_at = float(entry_r_at.get())
        v_r_iz = int(entry_v_r_iz.get());  r_iz = float(entry_r_iz.get()); d_r_iz = float(entry_d_r_iz.get())

        # COMPRESIÓN DEL PAQUETE UDP (Orden estricto de lectura para el Arduino)
        mensaje = (f"C,{figura},{ms_f_tri},{ms_diag},{ms_f_cr},{ms_lat},"
                   f"{v_t_ad},{t_ad},{v_t_dd},{t_dd},{a_t_d},{v_t_di},{t_di},{a_t_i},"
                   f"{v_c_ad},{c_ad},{v_c_de},{c_de},{d_c_de},{v_c_at},{c_at},{v_c_iz},{c_iz},{d_c_iz},"
                   f"{v_r_ad},{r_ad},{v_r_de},{r_de},{d_r_de},{v_r_at},{r_at},{v_r_iz},{r_iz},{d_r_iz},{t_esq}\n")
        
        sock.sendto(mensaje.encode('utf-8'), (ROBOT_IP, PORT))
        lbl_estado.config(text="✅ Cerebro Geométrico Sincronizado", fg="green")
    except ValueError:
        messagebox.showerror("Error", "Revisa los campos. Enteros para velocidad/tiempos y puntos para decimales.")

def iniciar_trazado():
    sock.sendto("G\n".encode('utf-8'), (ROBOT_IP, PORT))
    lbl_estado.config(text="▶️ Ejecutando dibujo en el robot...", fg="blue")

def detener_freno():
    sock.sendto("S\n".encode('utf-8'), (ROBOT_IP, PORT))
    lbl_estado.config(text="🚨 FRENO DE EMERGENCIA ACTIVADO 🚨", fg="red")

# --- INTERFAZ GRÁFICA ---
root = tk.Tk()
root.title("Mecanum Matrix - Calibración Quirúrgica")
root.geometry("620x750")
root.configure(padx=10, pady=5)

# Selector de figura y Esquinas
f_top = tk.Frame(root)
f_top.pack(fill="x", pady=5)
var_figura = tk.StringVar(value="1")
tk.Radiobutton(f_top, text="▲ Triángulo", variable=var_figura, value="1", font=("Arial", 10, "bold")).grid(row=0, column=0, padx=5)
tk.Radiobutton(f_top, text="■ Cuadrado", variable=var_figura, value="2", font=("Arial", 10, "bold")).grid(row=0, column=1, padx=5)
tk.Radiobutton(f_top, text="█ Rectángulo", variable=var_figura, value="3", font=("Arial", 10, "bold")).grid(row=0, column=2, padx=5)
tk.Label(f_top, text="Pausa Esquinas (ms):").grid(row=0, column=3, padx=10)
entry_t_esquinas = tk.Entry(f_top, width=6, justify="center"); entry_t_esquinas.insert(0, "500"); entry_t_esquinas.grid(row=0, column=4)

# 1. Tiempos Físicos de fricción
f_física = tk.LabelFrame(root, text="1. Constantes de Fricción del Suelo (ms por cada 1 CM real)", fg="blue", font=("Arial", 9, "bold"))
f_física.pack(fill="x", pady=3)
tk.Label(f_física, text="Triáng. Recto:").grid(row=0, column=0, padx=2); entry_ms_f_tri = tk.Entry(f_física, width=6, justify="center"); entry_ms_f_tri.insert(0, "21.0"); entry_ms_f_tri.grid(row=0, column=1, padx=5)
tk.Label(f_física, text="Triáng. Diag:").grid(row=0, column=2, padx=2); entry_ms_diag = tk.Entry(f_física, width=6, justify="center"); entry_ms_diag.insert(0, "35.0"); entry_ms_diag.grid(row=0, column=3, padx=5)
tk.Label(f_física, text="Cuad/Rect Recto:").grid(row=0, column=4, padx=2); entry_ms_f_cr = tk.Entry(f_física, width=6, justify="center"); entry_ms_f_cr.insert(0, "28.0"); entry_ms_f_cr.grid(row=0, column=5, padx=5)
tk.Label(f_física, text="Cuad/Rect Lateral:").grid(row=0, column=6, padx=2); entry_ms_lat = tk.Entry(f_física, width=6, justify="center"); entry_ms_lat.insert(0, "43.3"); entry_ms_lat.grid(row=0, column=7, padx=5)

# Auxiliar para crear filas de datos de forma compacta
def crear_fila_linea(frame, fila, texto, v_def, c_def, ang_def=None, txt_ang="Ángulo:"):
    tk.Label(frame, text=texto, font=("Arial", 9, "bold")).grid(row=fila, column=0, sticky="w", padx=5, pady=2)
    tk.Label(frame, text="Vel:").grid(row=fila, column=1)
    e_v = tk.Entry(frame, width=5, justify="center"); e_v.insert(0, v_def); e_v.grid(row=fila, column=2, padx=2)
    tk.Label(frame, text="cm:").grid(row=fila, column=3)
    e_c = tk.Entry(frame, width=5, justify="center"); e_c.insert(0, c_def); e_c.grid(row=fila, column=4, padx=2)
    e_a = None
    if ang_def is not None:
        tk.Label(frame, text=txt_ang).grid(row=fila, column=5)
        e_a = tk.Entry(frame, width=6, justify="center"); e_a.insert(0, ang_def); e_a.grid(row=fila, column=6, padx=2)
    return e_v, e_c, e_a

# 2. Panel Triángulo
f_tri = tk.LabelFrame(root, text="2. Configuración Lado a Lado: TRIÁNGULO", fg="#9900ff", font=("Arial", 9, "bold"))
f_tri.pack(fill="x", pady=3)
entry_v_t_ad, entry_t_ad, _ = crear_fila_linea(f_tri, 0, "L1 (Adelante)", "100", "25.0")
entry_v_t_dd, entry_t_dd, entry_a_t_d = crear_fila_linea(f_tri, 1, "L2 (Diag. Der)", "100", "35.0", "0.60")
entry_v_t_di, entry_t_di, entry_a_t_i = crear_fila_linea(f_tri, 2, "L3 (Diag. Izq)", "100", "35.0", "0.60")

# 3. Panel Cuadrado
f_cuad = tk.LabelFrame(root, text="3. Configuración Lado a Lado: CUADRADO", fg="green", font=("Arial", 9, "bold"))
f_cuad.pack(fill="x", pady=3)
entry_v_c_ad, entry_c_ad, _ = crear_fila_linea(f_cuad, 0, "L1 (Adelante)", "75", "20.0")
entry_v_c_de, entry_c_de, entry_d_c_de = crear_fila_linea(f_cuad, 1, "L2 (Lateral Der)", "75", "20.0", "0.0", "Deriva:")
entry_v_c_at, entry_c_at, _ = crear_fila_linea(f_cuad, 2, "L3 (Atrás)", "85", "20.0")
entry_v_c_iz, entry_c_iz, entry_d_c_iz = crear_fila_linea(f_cuad, 3, "L4 (Lateral Izq)", "75", "20.0", "0.0", "Deriva:")

# 4. Panel Rectángulo
f_rect = tk.LabelFrame(root, text="4. Configuración Lado a Lado: RECTÁNGULO", fg="#b38f00", font=("Arial", 9, "bold"))
f_rect.pack(fill="x", pady=3)
entry_v_r_ad, entry_r_ad, _ = crear_fila_linea(f_rect, 0, "L1 (Adelante)", "75", "28.0")
entry_v_r_de, entry_r_de, entry_d_r_de = crear_fila_linea(f_rect, 1, "L2 (Lateral Der)", "75", "12.0", "0.0", "Deriva:")
entry_v_r_at, entry_r_at, _ = crear_fila_linea(f_rect, 2, "L3 (Atrás)", "85", "28.0")
entry_v_r_iz, entry_r_iz, entry_d_r_iz = crear_fila_linea(f_rect, 3, "L4 (Lateral Izq)", "75", "12.0", "0.0", "Deriva:")

# Botones de control técnico
tk.Button(root, text="💾 SINCRONIZAR VARIABLES POR WI-FI", command=enviar_configuracion, bg="#e6f2ff", font=("Arial", 11, "bold")).pack(fill="x", pady=5)
f_btn = tk.Frame(root)
f_btn.pack(fill="x", pady=3)
tk.Button(f_btn, text="⏹️ FRENAR MOTORES", command=detener_freno, bg="#ff4d4d", fg="white", font=("Arial", 10, "bold"), height=2, width=28).grid(row=0, column=0, padx=10)
tk.Button(f_btn, text="▶️ DIBUJAR AHORA", command=iniciar_trazado, bg="#4da6ff", fg="white", font=("Arial", 10, "bold"), height=2, width=28).grid(row=0, column=1, padx=10)

lbl_estado = tk.Label(root, text="Panel operativo. Ajusta variables y sincroniza.", font=("Arial", 10, "italic"), fg="gray")
lbl_estado.pack(pady=5)

root.mainloop()