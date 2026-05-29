import tkinter as tk
from tkinter import messagebox
import socket

ROBOT_IP = "192.168.4.1" 
PORT = 8080
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def guardar_cambios():
    try:
        base_p = float(entry_base_p.get())
        base_i = float(entry_base_i.get())
        base_d = float(entry_base_d.get())
        
        v_p = float(entry_v_p.get()); v_i = float(entry_v_i.get()); v_d = float(entry_v_d.get())
        a_p = float(entry_a_p.get()); a_i = float(entry_a_i.get()); a_d = float(entry_a_d.get())
        r_p = float(entry_r_p.get()); r_i = float(entry_r_i.get()); r_d = float(entry_r_d.get())
        
        vel = int(entry_vel.get()) 
        t_barrido = int(entry_barrido.get()) 
        
        mensaje = f"F,{vel},{t_barrido},{base_p},{base_i},{base_d},{v_p},{v_i},{v_d},{a_p},{a_i},{a_d},{r_p},{r_i},{r_d}\n"
        sock.sendto(mensaje.encode('utf-8'), (ROBOT_IP, PORT))
        lbl_estado.config(text=f"✅ Datos PID Completos enviados", fg="green")
    except ValueError:
        messagebox.showerror("Error", "Usa puntos para los decimales.")

def parar_robot(event=None):
    sock.sendto(b"S\n", (ROBOT_IP, PORT))
    lbl_estado.config(text="🚨 ¡ROBOT DETENIDO! 🚨", fg="red")
    
def reanudar_robot():
    sock.sendto(b"R\n", (ROBOT_IP, PORT))
    lbl_estado.config(text="▶️ Robot en marcha (PID)", fg="blue")

def ir_adelante_press(event):
    sock.sendto(b"A\n", (ROBOT_IP, PORT)) 
    lbl_estado.config(text="⬆️ Moviendo Adelante...", fg="#9900ff")

def girar_derecha_press(event):
    sock.sendto(b"D\n", (ROBOT_IP, PORT))
    lbl_estado.config(text="🔄 Girando a la derecha...", fg="#ff9900")

# ================= DISEÑO DE LA INTERFAZ =================
root = tk.Tk()
root.title("Fuzzy PID - Integral Condicional")
root.geometry("620x650")
root.configure(padx=10, pady=10)

tk.Label(root, text="🚀 TELEMETRÍA FUZZY P.I.D", font=("Arial", 12, "bold")).pack()

f_vel = tk.Frame(root)
f_vel.pack(pady=5)
tk.Label(f_vel, text="Vel. Base:").grid(row=0, column=0, padx=5); entry_vel = tk.Entry(f_vel, width=6, justify="center"); entry_vel.insert(0, "150"); entry_vel.grid(row=0, column=1)
tk.Label(f_vel, text="Tiempo Barrido (ms):").grid(row=0, column=2, padx=15); entry_barrido = tk.Entry(f_vel, width=6, justify="center"); entry_barrido.insert(0, "500"); entry_barrido.grid(row=0, column=3)

# Cajas de valores
def crear_fila(frame, color, txt_p, txt_i, txt_d, def_p, def_i, def_d):
    f = tk.LabelFrame(root, text=txt_p.split(":")[0], fg=color, font=("Arial", 10, "bold"))
    f.pack(fill="x", pady=5, ipadx=5, ipady=5)
    
    tk.Label(f, text="Kp:").grid(row=0, column=0, padx=2); e_p = tk.Entry(f, width=8, justify="center"); e_p.insert(0, def_p); e_p.grid(row=0, column=1)
    tk.Label(f, text="Ki:").grid(row=0, column=2, padx=15); e_i = tk.Entry(f, width=8, justify="center"); e_i.insert(0, def_i); e_i.grid(row=0, column=3)
    tk.Label(f, text="Kd:").grid(row=0, column=4, padx=15); e_d = tk.Entry(f, width=8, justify="center"); e_d.insert(0, def_d); e_d.grid(row=0, column=5)
    return e_p, e_i, e_d

entry_base_p, entry_base_i, entry_base_d = crear_fila(root, "blue", "1. PID Base", "", "", "0.2", "0.001", "5.0")
entry_v_p, entry_v_i, entry_v_d = crear_fila(root, "green", "2. Zona Verde (Recta)", "", "", "0.4", "1.0", "1.0")
entry_a_p, entry_a_i, entry_a_d = crear_fila(root, "#b38f00", "3. Zona Amarilla (Curva)", "", "", "1.0", "0.0", "2.5")
entry_r_p, entry_r_i, entry_r_d = crear_fila(root, "red", "4. Zona Roja (Peligro)", "", "", "3.2", "0.0", "1.7")

tk.Button(root, text="💾 ACTUALIZAR CEREBRO FUZZY", command=guardar_cambios, bg="#d9f2d9", font=("Arial", 11, "bold")).pack(fill="x", pady=10)

f_ctrl = tk.Frame(root)
f_ctrl.pack(fill="x", pady=5)
tk.Button(f_ctrl, text="⏹️ PARAR", command=parar_robot, bg="#ff4d4d", fg="white", font=("Arial", 10, "bold"), height=2, width=20).grid(row=0, column=0, padx=25)
tk.Button(f_ctrl, text="▶️ REANUDAR", command=reanudar_robot, bg="#4da6ff", fg="white", font=("Arial", 10, "bold"), height=2, width=20).grid(row=0, column=1, padx=25)

btn_adelante = tk.Button(f_ctrl, text="⬆️ SOLO ADELANTE", bg="#cc99ff", font=("Arial", 10, "bold"), height=2, width=48)
btn_adelante.grid(row=1, column=0, columnspan=2, pady=5); btn_adelante.bind('<ButtonPress-1>', ir_adelante_press); btn_adelante.bind('<ButtonRelease-1>', parar_robot)

btn_der = tk.Button(f_ctrl, text="↪️ GIRAR DER.", bg="#ffcc00", font=("Arial", 10, "bold"), height=2, width=48)
btn_der.grid(row=2, column=0, columnspan=2, pady=5); btn_der.bind('<ButtonPress-1>', girar_derecha_press); btn_der.bind('<ButtonRelease-1>', parar_robot)

lbl_estado = tk.Label(root, text="Esperando instrucciones...", font=("Arial", 10, "bold")); lbl_estado.pack(pady=5)

root.mainloop()