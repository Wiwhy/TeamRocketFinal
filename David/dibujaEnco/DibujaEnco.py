import tkinter as tk
from tkinter import messagebox
import socket

# --- CONFIGURACIÓN WI-FI ---
ROBOT_IP = "192.168.4.1" 
PORT = 8080
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def enviar_configuracion():
    try:
        fig = var_figura.get()
        pwm = int(entry_pwm.get())
        pausa = int(entry_pausa.get())
        
        tk_c_r = int(entry_c_r.get())
        tk_c_l = int(entry_c_l.get())
        
        tk_r_r = int(entry_r_r.get())
        tk_r_l = int(entry_r_l.get())
        
        # Nuevas variables independientes para el triángulo
        tk_tri_1 = int(entry_tri_1.get())
        tk_tri_2 = int(entry_tri_2.get())
        tk_tri_3 = int(entry_tri_3.get())
        tri_r1 = float(entry_tri_r1.get())
        tri_r2 = float(entry_tri_r2.get())
        
        lat_fl = float(entry_lat_fl.get())
        lat_fr = float(entry_lat_fr.get())
        lat_bl = float(entry_lat_bl.get())
        lat_br = float(entry_lat_br.get())

        # Formato actualizado (17 valores): C,fig,pwm,pausa,tk_c_r,tk_c_l,tk_r_r,tk_r_l,tk_tri_1,tk_tri_2,tk_tri_3,tri_r1,tri_r2,lat_fl,lat_fr,lat_bl,lat_br
        msg = f"C,{fig},{pwm},{pausa},{tk_c_r},{tk_c_l},{tk_r_r},{tk_r_l},{tk_tri_1},{tk_tri_2},{tk_tri_3},{tri_r1},{tri_r2},{lat_fl},{lat_fr},{lat_bl},{lat_br}\n"
        
        sock.sendto(msg.encode('utf-8'), (ROBOT_IP, PORT))
        lbl_estado.config(text=f"✅ Datos en memoria del robot (Figura {fig}, PWM {pwm})", fg="green")
    except ValueError:
        messagebox.showerror("Error", "Revisa que todos los campos sean números válidos.")

def ejecutar_figura():
    sock.sendto("G\n".encode('utf-8'), (ROBOT_IP, PORT))
    lbl_estado.config(text="▶️ Ejecutando Figura Automática...", fg="blue")

def freno_emergencia():
    sock.sendto("S\n".encode('utf-8'), (ROBOT_IP, PORT))
    lbl_estado.config(text="🚨 FRENADO DE EMERGENCIA ACTIVADO 🚨", fg="red")

# --- INTERFAZ GRÁFICA ---
root = tk.Tk()
root.title("Panel de Control Geométrico por Ticks")
root.geometry("640x550")
root.configure(padx=15, pady=10)

# --- 1. AJUSTES GLOBALES ---
f_glob = tk.LabelFrame(root, text="1. Parámetros Globales", font=("Arial", 10, "bold"), fg="blue")
f_glob.pack(fill="x", pady=5)
tk.Label(f_glob, text="Potencia Global (PWM 0-255):").grid(row=0, column=0, padx=5, pady=5)
entry_pwm = tk.Entry(f_glob, width=8, justify="center"); entry_pwm.insert(0, "250"); entry_pwm.grid(row=0, column=1)
tk.Label(f_glob, text="Pausa esquinas (ms):").grid(row=0, column=2, padx=15)
entry_pausa = tk.Entry(f_glob, width=8, justify="center"); entry_pausa.insert(0, "500"); entry_pausa.grid(row=0, column=3)

var_figura = tk.IntVar(value=1)
tk.Radiobutton(f_glob, text="▲ Triángulo", variable=var_figura, value=1).grid(row=1, column=0)
tk.Radiobutton(f_glob, text="■ Cuadrado", variable=var_figura, value=2).grid(row=1, column=1)
tk.Radiobutton(f_glob, text="█ Rectángulo", variable=var_figura, value=3).grid(row=1, column=2)

# --- 2. CONFIGURACIÓN CUADRADO Y RECTÁNGULO ---
f_cr = tk.LabelFrame(root, text="2. Ticks para Cuadrado y Rectángulo", font=("Arial", 10, "bold"), fg="green")
f_cr.pack(fill="x", pady=5)
tk.Label(f_cr, text="Cuadrado - Lados Rectos (Ticks):").grid(row=0, column=0, sticky="w")
entry_c_r = tk.Entry(f_cr, width=8, justify="center"); entry_c_r.insert(0, "840"); entry_c_r.grid(row=0, column=1, padx=5)
tk.Label(f_cr, text="Lados Laterales (Ticks):").grid(row=0, column=2, sticky="w")
entry_c_l = tk.Entry(f_cr, width=8, justify="center"); entry_c_l.insert(0, "1000"); entry_c_l.grid(row=0, column=3)

tk.Label(f_cr, text="Rectángulo - Lado Largo (Ticks):").grid(row=1, column=0, sticky="w")
entry_r_r = tk.Entry(f_cr, width=8, justify="center"); entry_r_r.insert(0, "1200"); entry_r_r.grid(row=1, column=1, padx=5, pady=5)
tk.Label(f_cr, text="Lado Corto Lateral (Ticks):").grid(row=1, column=2, sticky="w")
entry_r_l = tk.Entry(f_cr, width=8, justify="center"); entry_r_l.insert(0, "600"); entry_r_l.grid(row=1, column=3)

f_der = tk.LabelFrame(f_cr, text="Corrección de Deriva Lateral (% Velocidad por Rueda)", fg="gray")
f_der.grid(row=2, column=0, columnspan=4, pady=5, sticky="we")
tk.Label(f_der, text="FL:").grid(row=0, column=0); entry_lat_fl = tk.Entry(f_der, width=5); entry_lat_fl.insert(0, "100.0"); entry_lat_fl.grid(row=0, column=1)
tk.Label(f_der, text="FR:").grid(row=0, column=2); entry_lat_fr = tk.Entry(f_der, width=5); entry_lat_fr.insert(0, "100.0"); entry_lat_fr.grid(row=0, column=3)
tk.Label(f_der, text="BL:").grid(row=0, column=4); entry_lat_bl = tk.Entry(f_der, width=5); entry_lat_bl.insert(0, "100.0"); entry_lat_bl.grid(row=0, column=5)
tk.Label(f_der, text="BR:").grid(row=0, column=6); entry_lat_br = tk.Entry(f_der, width=5); entry_lat_br.insert(0, "100.0"); entry_lat_br.grid(row=0, column=7)

# --- 3. CONFIGURACIÓN TRIÁNGULO INDEPENDIENTE ---
f_tri = tk.LabelFrame(root, text="3. Ticks y Ángulos para Triángulo Equilátero", font=("Arial", 10, "bold"), fg="purple")
f_tri.pack(fill="x", pady=5)

tk.Label(f_tri, text="Lado 1 (Avance Recto) Ticks:").grid(row=0, column=0, sticky="w", pady=2)
entry_tri_1 = tk.Entry(f_tri, width=8, justify="center"); entry_tri_1.insert(0, "1000"); entry_tri_1.grid(row=0, column=1)

tk.Label(f_tri, text="Lado 2 (Diag. Der) Ticks:").grid(row=1, column=0, sticky="w", pady=2)
entry_tri_2 = tk.Entry(f_tri, width=8, justify="center"); entry_tri_2.insert(0, "1366"); entry_tri_2.grid(row=1, column=1)
tk.Label(f_tri, text="Rueda Lenta (%):").grid(row=1, column=2, sticky="e", padx=5)
entry_tri_r1 = tk.Entry(f_tri, width=6, justify="center"); entry_tri_r1.insert(0, "26.8"); entry_tri_r1.grid(row=1, column=3)

tk.Label(f_tri, text="Lado 3 (Diag. Izq) Ticks:").grid(row=2, column=0, sticky="w", pady=2)
entry_tri_3 = tk.Entry(f_tri, width=8, justify="center"); entry_tri_3.insert(0, "1366"); entry_tri_3.grid(row=2, column=1)
tk.Label(f_tri, text="Rueda Lenta (%):").grid(row=2, column=2, sticky="e", padx=5)
entry_tri_r2 = tk.Entry(f_tri, width=6, justify="center"); entry_tri_r2.insert(0, "26.8"); entry_tri_r2.grid(row=2, column=3)

# --- BOTONES ---
f_botones = tk.Frame(root)
f_botones.pack(fill="x", pady=10)

tk.Button(f_botones, text="💾 1. ENVIAR DATOS", command=enviar_configuracion, bg="#e6f2ff", font=("Arial", 10, "bold"), height=2, width=22).grid(row=0, column=0, padx=5)
tk.Button(f_botones, text="▶️ 2. DIBUJAR AHORA", command=ejecutar_figura, bg="#4da6ff", fg="white", font=("Arial", 10, "bold"), height=2, width=22).grid(row=0, column=1, padx=5)
tk.Button(f_botones, text="⏹️ FRENO", command=freno_emergencia, bg="#ff4d4d", fg="white", font=("Arial", 10, "bold"), height=2, width=12).grid(row=0, column=2, padx=5)

lbl_estado = tk.Label(root, text="Esperando conexión...", fg="gray")
lbl_estado.pack(pady=5)

root.mainloop()