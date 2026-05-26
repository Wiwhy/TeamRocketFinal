import tkinter as tk
from tkinter import messagebox
import socket

ROBOT_IP = "192.168.4.1" 
PORT = 8080
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def guardar_cambios():
    try:
        base_p = float(entry_base_p.get())
        base_d = float(entry_base_d.get())
        v_p = float(entry_v_p.get())
        v_d = float(entry_v_d.get())
        a_p = float(entry_a_p.get())
        a_d = float(entry_a_d.get())
        r_p = float(entry_r_p.get())
        r_d = float(entry_r_d.get())
        vel = int(entry_vel.get()) 
        t_barrido = int(entry_barrido.get()) 
        
        mensaje = f"F,{vel},{t_barrido},{base_p},{base_d},{v_p},{v_d},{a_p},{a_d},{r_p},{r_d}\n"
        sock.sendto(mensaje.encode('utf-8'), (ROBOT_IP, PORT))
        lbl_estado.config(text=f"✅ Datos enviados! Vel: {vel} | Barrido: {t_barrido}ms", fg="green")
    except ValueError:
        messagebox.showerror("Error", "Usa puntos para los decimales y enteros para vel/barrido.")

def parar_robot(event=None):
    sock.sendto("S\n".encode('utf-8'), (ROBOT_IP, PORT))
    lbl_estado.config(text="🚨 ¡ROBOT DETENIDO! 🚨", fg="red")
    
def reanudar_robot():
    sock.sendto("R\n".encode('utf-8'), (ROBOT_IP, PORT))
    lbl_estado.config(text="▶️ Robot en marcha (PID)", fg="blue")

def ir_adelante_press(event):
    sock.sendto("A\n".encode('utf-8'), (ROBOT_IP, PORT)) 
    lbl_estado.config(text="⬆️ Moviendo Adelante...", fg="#9900ff")

def girar_derecha_press(event):
    sock.sendto("D\n".encode('utf-8'), (ROBOT_IP, PORT))
    lbl_estado.config(text="🔄 Girando a la derecha...", fg="#ff9900")

# ================= DISEÑO DE LA INTERFAZ =================
root = tk.Tk()
root.title("Fuzzy PID - MODO DIOS + MANDO")
root.geometry("480x650")
root.configure(padx=10, pady=10)

tk.Label(root, text=" TELEMETRÍA FUZZY", font=("Arial", 12, "bold")).pack()

f_vel = tk.Frame(root)
f_vel.pack(pady=5)
tk.Label(f_vel, text="Vel. Base:", font=("Arial", 10, "bold")).grid(row=0, column=0, padx=5)
entry_vel = tk.Entry(f_vel, width=6, justify="center"); entry_vel.insert(0, "150"); entry_vel.grid(row=0, column=1)

# 🚨 NUEVA CASILLA EN LA INTERFAZ
tk.Label(f_vel, text="Tiempo Barrido (ms):", font=("Arial", 10, "bold")).grid(row=0, column=2, padx=15)
entry_barrido = tk.Entry(f_vel, width=6, justify="center"); entry_barrido.insert(0, "500"); entry_barrido.grid(row=0, column=3)

f_base = tk.LabelFrame(root, text="1. PID Base (Fuerza Bruta)", fg="blue", font=("Arial", 10, "bold"))
f_base.pack(fill="x", pady=5, ipadx=5, ipady=5)
tk.Label(f_base, text="Kp Base:").grid(row=0, column=0, padx=5); entry_base_p = tk.Entry(f_base, width=8, justify="center"); entry_base_p.insert(0, "0.2"); entry_base_p.grid(row=0, column=1)
tk.Label(f_base, text="Kd Base:").grid(row=0, column=2, padx=15); entry_base_d = tk.Entry(f_base, width=8, justify="center"); entry_base_d.insert(0, "5.0"); entry_base_d.grid(row=0, column=3)

f_verde = tk.LabelFrame(root, text="2. Zona Verde (Recta)", fg="green", font=("Arial", 10, "bold"))
f_verde.pack(fill="x", pady=5, ipadx=5, ipady=5)
tk.Label(f_verde, text="Mult. Kp:").grid(row=0, column=0, padx=5); entry_v_p = tk.Entry(f_verde, width=8, justify="center"); entry_v_p.insert(0, "0.4"); entry_v_p.grid(row=0, column=1)
tk.Label(f_verde, text="Mult. Kd:").grid(row=0, column=2, padx=15); entry_v_d = tk.Entry(f_verde, width=8, justify="center"); entry_v_d.insert(0, "1.0"); entry_v_d.grid(row=0, column=3)

f_ama = tk.LabelFrame(root, text="3. Zona Amarilla (Curva)", fg="#b38f00", font=("Arial", 10, "bold"))
f_ama.pack(fill="x", pady=5, ipadx=5, ipady=5)
tk.Label(f_ama, text="Mult. Kp:").grid(row=0, column=0, padx=5); entry_a_p = tk.Entry(f_ama, width=8, justify="center"); entry_a_p.insert(0, "1.0"); entry_a_p.grid(row=0, column=1)
tk.Label(f_ama, text="Mult. Kd:").grid(row=0, column=2, padx=15); entry_a_d = tk.Entry(f_ama, width=8, justify="center"); entry_a_d.insert(0, "2.5"); entry_a_d.grid(row=0, column=3)

f_rojo = tk.LabelFrame(root, text="4. Zona Roja (Peligro)", fg="red", font=("Arial", 10, "bold"))
f_rojo.pack(fill="x", pady=5, ipadx=5, ipady=5)
tk.Label(f_rojo, text="Mult. Kp:").grid(row=0, column=0, padx=5); entry_r_p = tk.Entry(f_rojo, width=8, justify="center"); entry_r_p.insert(0, "3.2"); entry_r_p.grid(row=0, column=1)
tk.Label(f_rojo, text="Mult. Kd:").grid(row=0, column=2, padx=15); entry_r_d = tk.Entry(f_rojo, width=8, justify="center"); entry_r_d.insert(0, "1.7"); entry_r_d.grid(row=0, column=3)

tk.Button(root, text="💾 ACTUALIZAR CEREBRO FUZZY", command=guardar_cambios, bg="#d9f2d9", font=("Arial", 11, "bold")).pack(fill="x", pady=5)

f_ctrl = tk.Frame(root)
f_ctrl.pack(fill="x", pady=5)

tk.Button(f_ctrl, text="⏹️ PARAR", command=parar_robot, bg="#ff4d4d", fg="white", font=("Arial", 10, "bold"), height=2, width=15).grid(row=0, column=0, padx=10)
tk.Button(f_ctrl, text="▶️ REANUDAR", command=reanudar_robot, bg="#4da6ff", fg="white", font=("Arial", 10, "bold"), height=2, width=15).grid(row=0, column=1, padx=10)

btn_adelante = tk.Button(f_ctrl, text="⬆️ SOLO ADELANTE (Mantener)", bg="#cc99ff", font=("Arial", 10, "bold"), height=2, width=33)
btn_adelante.grid(row=1, column=0, columnspan=2, pady=5)
btn_adelante.bind('<ButtonPress-1>', ir_adelante_press)
btn_adelante.bind('<ButtonRelease-1>', parar_robot)

btn_der = tk.Button(f_ctrl, text="↪️ GIRAR DER. (Mantener)", bg="#ffcc00", font=("Arial", 10, "bold"), height=2, width=33)
btn_der.grid(row=2, column=0, columnspan=2, pady=5)
btn_der.bind('<ButtonPress-1>', girar_derecha_press)
btn_der.bind('<ButtonRelease-1>', parar_robot)

lbl_estado = tk.Label(root, text="Esperando instrucciones...", font=("Arial", 10, "bold"))
lbl_estado.pack(pady=5)

root.mainloop()