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
        
        # 2. Parámetros del Triángulo (Velocidad y Milisegundos)
        v_t_ad = int(entry_v_t_ad.get());  ms_t_ad = int(entry_ms_t_ad.get())
        v_t_dd = int(entry_v_t_dd.get());  ms_t_dd = int(entry_ms_t_dd.get()); a_t_d = float(entry_a_t_d.get())
        v_t_di = int(entry_v_t_di.get());  ms_t_di = int(entry_ms_t_di.get()); a_t_i = float(entry_a_t_i.get())
        
        # 3. Parámetros del Cuadrado (Velocidad y Milisegundos)
        v_c_ad = int(entry_v_c_ad.get());  ms_c_ad = int(entry_ms_c_ad.get())
        v_c_de = int(entry_v_c_de.get());  ms_c_de = int(entry_ms_c_de.get()); d_c_de = float(entry_d_c_de.get())
        v_c_at = int(entry_v_c_at.get());  ms_c_at = int(entry_ms_c_at.get())
        v_c_iz = int(entry_v_c_iz.get());  ms_c_iz = int(entry_ms_c_iz.get()); d_c_iz = float(entry_d_c_iz.get())
        
        # 4. Parámetros del Rectángulo (Velocidad y Milisegundos)
        v_r_ad = int(entry_v_r_ad.get());  ms_r_ad = int(entry_ms_r_ad.get())
        v_r_de = int(entry_v_r_de.get());  ms_r_de = int(entry_ms_r_de.get()); d_r_de = float(entry_d_r_de.get())
        v_r_at = int(entry_v_r_at.get());  ms_r_at = int(entry_ms_r_at.get())
        v_r_iz = int(entry_v_r_iz.get());  ms_r_iz = int(entry_ms_r_iz.get()); d_r_iz = float(entry_d_r_iz.get())

        # PAQUETE UDP SIMPLIFICADO: Enviamos directamente los ms acumulados
        mensaje = (f"C,{figura},"
                   f"{v_t_ad},{ms_t_ad},{v_t_dd},{ms_t_dd},{a_t_d},{v_t_di},{ms_t_di},{a_t_i},"
                   f"{v_c_ad},{ms_c_ad},{v_c_de},{ms_c_de},{d_c_de},{v_c_at},{ms_c_at},{v_c_iz},{ms_c_iz},{d_c_iz},"
                   f"{v_r_ad},{ms_r_ad},{v_r_de},{ms_r_de},{d_r_de},{v_r_at},{ms_r_at},{v_r_iz},{ms_r_iz},{d_r_iz},{t_esq}\n")
        
        sock.sendto(mensaje.encode('utf-8'), (ROBOT_IP, PORT))
        lbl_estado.config(text="✅ Variables de Tiempo Sincronizadas", fg="green")
    except ValueError:
        messagebox.showerror("Error", "Revisa los campos. Usa enteros para velocidades y milisegundos.")

def iniciar_trazado():
    sock.sendto("G\n".encode('utf-8'), (ROBOT_IP, PORT))
    lbl_estado.config(text="▶️ Ejecutando dibujo por tiempos...", fg="blue")

def detener_freno():
    sock.sendto("S\n".encode('utf-8'), (ROBOT_IP, PORT))
    lbl_estado.config(text="🚨 FRENO DE EMERGENCY ACTIVADO 🚨", fg="red")

# --- INTERFAZ GRÁFICA ---
root = tk.Tk()
root.title("Mecanum Matrix - Control por Tiempos Directos")
root.geometry("620x700")
root.configure(padx=10, pady=5)

f_top = tk.Frame(root)
f_top.pack(fill="x", pady=5)
var_figura = tk.StringVar(value="1")
tk.Radiobutton(f_top, text="▲ Triángulo", variable=var_figura, value="1", font=("Arial", 10, "bold")).grid(row=0, column=0, padx=5)
tk.Radiobutton(f_top, text="■ Cuadrado", variable=var_figura, value="2", font=("Arial", 10, "bold")).grid(row=0, column=1, padx=5)
tk.Radiobutton(f_top, text="█ Rectángulo", variable=var_figura, value="3", font=("Arial", 10, "bold")).grid(row=0, column=2, padx=5)
tk.Label(f_top, text="Pausa Esquinas (ms):").grid(row=0, column=3, padx=10)
entry_t_esquinas = tk.Entry(f_top, width=6, justify="center"); entry_t_esquinas.insert(0, "500"); entry_t_esquinas.grid(row=0, column=4)

def crear_fila_tiempo(frame, fila, texto, v_def, ms_def, ang_def=None, txt_ang="Ángulo:"):
    tk.Label(frame, text=texto, font=("Arial", 9, "bold")).grid(row=fila, column=0, sticky="w", padx=5, pady=2)
    tk.Label(frame, text="Vel:").grid(row=fila, column=1)
    e_v = tk.Entry(frame, width=5, justify="center"); e_v.insert(0, v_def); e_v.grid(row=fila, column=2, padx=2)
    tk.Label(frame, text="ms:").grid(row=fila, column=3)
    e_ms = tk.Entry(frame, width=6, justify="center"); e_ms.insert(0, ms_def); e_ms.grid(row=fila, column=4, padx=2)
    e_a = None
    if ang_def is not None:
        tk.Label(frame, text=txt_ang).grid(row=fila, column=5)
        e_a = tk.Entry(frame, width=6, justify="center"); e_a.insert(0, ang_def); e_a.grid(row=fila, column=6, padx=2)
    return e_v, e_ms, e_a

# Panels
f_tri = tk.LabelFrame(root, text="1. Configuración por Milisegundos: TRIÁNGULO", fg="#9900ff", font=("Arial", 9, "bold"))
f_tri.pack(fill="x", pady=5)
entry_v_t_ad, entry_ms_t_ad, _ = crear_fila_tiempo(f_tri, 0, "L1 (Adelante)", "100", "525")
entry_v_t_dd, entry_ms_t_dd, entry_a_t_d = crear_fila_tiempo(f_tri, 1, "L2 (Diag. Der)", "100", "1225", "0.60")
entry_v_t_di, entry_ms_t_di, entry_a_t_i = crear_fila_tiempo(f_tri, 2, "L3 (Diag. Izq)", "100", "1225", "0.60")

f_cuad = tk.LabelFrame(root, text="2. Configuración por Milisegundos: CUADRADO", fg="green", font=("Arial", 9, "bold"))
f_cuad.pack(fill="x", pady=5)
entry_v_c_ad, entry_ms_c_ad, _ = crear_fila_tiempo(f_cuad, 0, "L1 (Adelante)", "75", "560")
entry_v_c_de, entry_ms_c_de, entry_d_c_de = crear_fila_tiempo(f_cuad, 1, "L2 (Lateral Der)", "75", "866", "0.0", "Deriva:")
entry_v_c_at, entry_ms_c_at, _ = crear_fila_tiempo(f_cuad, 2, "L3 (Atrás)", "85", "560")
entry_v_c_iz, entry_ms_c_iz, entry_d_c_iz = crear_fila_tiempo(f_cuad, 3, "L4 (Lateral Izq)", "75", "866", "0.0", "Deriva:")

f_rect = tk.LabelFrame(root, text="3. Configuración por Milisegundos: RECTÁNGULO", fg="#b38f00", font=("Arial", 9, "bold"))
f_rect.pack(fill="x", pady=5)
entry_v_r_ad, entry_ms_r_ad, _ = crear_fila_tiempo(f_rect, 0, "L1 (Adelante)", "75", "784")
entry_v_r_de, entry_ms_r_de, entry_d_r_de = crear_fila_tiempo(f_rect, 1, "L2 (Lateral Der)", "75", "520", "0.0", "Deriva:")
entry_v_r_at, entry_ms_r_at, _ = crear_fila_tiempo(f_rect, 2, "L3 (Atrás)", "85", "784")
entry_v_r_iz, entry_ms_r_iz, entry_d_r_iz = crear_fila_tiempo(f_rect, 3, "L4 (Lateral Izq)", "75", "520", "0.0", "Deriva:")

tk.Button(root, text="💾 SINCRO TIEMPOS POR WI-FI", command=enviar_configuracion, bg="#e6f2ff", font=("Arial", 11, "bold")).pack(fill="x", pady=5)
f_btn = tk.Frame(root)
f_btn.pack(fill="x", pady=3)
tk.Button(f_btn, text="⏹️ FRENAR MOTORES", command=detener_freno, bg="#ff4d4d", fg="white", font=("Arial", 10, "bold"), height=2, width=28).grid(row=0, column=0, padx=10)
tk.Button(f_btn, text="▶️ DIBUJAR AHORA", command=iniciar_trazado, bg="#4da6ff", fg="white", font=("Arial", 10, "bold"), height=2, width=28).grid(row=0, column=1, padx=10)

lbl_estado = tk.Label(root, text="Listo para calibrar por milisegundos directos.", font=("Arial", 10, "italic"), fg="gray")
lbl_estado.pack(pady=5)

root.mainloop()