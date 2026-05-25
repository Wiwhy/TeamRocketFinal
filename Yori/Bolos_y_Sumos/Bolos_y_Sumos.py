import tkinter as tk
from tkinter import messagebox
import socket
import time

class GCS_Robot:
    def __init__(self, root):
        self.root = root
        self.root.title("GCS - Panel Dual de Batalla (Estructura Original)")
        self.root.geometry("1000x700") # Ventana más ancha para las dos columnas

        # Red UDP
        self.ip = "192.168.4.1"
        self.port = 8080
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        # ---------------------------------------------------------
        # ZONA 1: CONTROL AUTÓNOMO Y MODO
        # ---------------------------------------------------------
        f_auto = tk.LabelFrame(root, text="⚔️ ESTADO Y BATALLA AUTÓNOMA", font=("Arial", 12, "bold"), padx=10, pady=10, bg="#f0f0f0")
        f_auto.pack(fill="x", padx=10, pady=5)
        
        self.modo_var = tk.IntVar(value=1)
        tk.Radiobutton(f_auto, text="Modo SUMO", variable=self.modo_var, value=1, font=("Arial", 11, "bold"), fg="red", bg="#f0f0f0", command=self.enviar_modo).pack(side=tk.LEFT, padx=20)
        tk.Radiobutton(f_auto, text="Modo TIRABOLOS", variable=self.modo_var, value=0, font=("Arial", 11, "bold"), fg="blue", bg="#f0f0f0", command=self.enviar_modo).pack(side=tk.LEFT, padx=20)
        
        tk.Button(f_auto, text="▶ INICIAR BATALLA", font=("Arial", 12, "bold"), bg="lightgreen", width=15, command=lambda: self.enviar_comando(1)).pack(side=tk.LEFT, padx=10)
        tk.Button(f_auto, text="🛑 DETENER EMERGENCIA", font=("Arial", 12, "bold"), bg="salmon", width=22, command=lambda: self.enviar_comando(0)).pack(side=tk.LEFT, padx=10)

        # Contenedor central para las dos columnas
        f_main = tk.Frame(root)
        f_main.pack(fill="both", expand=True, padx=10, pady=5)

        # ---------------------------------------------------------
        # COLUMNA IZQUIERDA: CONFIGURACIONES COMPARTIDAS
        # ---------------------------------------------------------
        f_comp = tk.LabelFrame(f_main, text="⚙️ CONFIGURACIONES COMPARTIDAS", font=("Arial", 11, "bold"), padx=10, pady=10)
        f_comp.pack(side=tk.LEFT, fill="both", expand=True, padx=(0, 5))

        self.param_compartidos = [
            ("T_RECTO", "Tiempo Contramarcha Recto", "70"),
            ("T_GIRO", "Tiempo Contramarcha Giro", "50"),
            ("P_LINEA", "Pausa en Línea Frontal", "0"),
            ("V_IN_RET", "Velocidad Inicio Retroceso", "255"),
            ("V_IN_AV", "Velocidad Inicio Avance", "200"),
            ("V_ATAQUE", "Velocidad Ataque", "255"),
            ("V_RET", "Velocidad Retroceso", "200"),
            ("TM_BUSCA", "Timeout Búsqueda (ms)", "5000"),
            ("D_PRE", "Delay Pre-Ataque (ms)", "10"),
            ("D_POST", "Delay Post-Retroceso", "200")
        ]

        self.cajas_comp = {}
        row = 0
        for clave, titulo, def_val in self.param_compartidos:
            tk.Label(f_comp, text=titulo, font=("Arial", 9)).grid(row=row, column=0, sticky="w", pady=4)
            ent = tk.Entry(f_comp, width=8, justify="center")
            ent.insert(0, def_val)
            ent.grid(row=row, column=1, padx=10)
            tk.Button(f_comp, text="Enviar", command=lambda c=clave: self.enviar_compartido(c)).grid(row=row, column=2)
            self.cajas_comp[clave] = ent
            row += 1

        # ---------------------------------------------------------
        # COLUMNA DERECHA: CONFIGURACIONES ESPECÍFICAS
        # ---------------------------------------------------------
        f_esp = tk.LabelFrame(f_main, text="⚙️ PERFILES ESPECÍFICOS", font=("Arial", 11, "bold"), padx=10, pady=10)
        f_esp.pack(side=tk.RIGHT, fill="both", expand=True, padx=(5, 0))

        tk.Label(f_esp, text="PARÁMETRO", font=("Arial", 9, "bold")).grid(row=0, column=0, sticky="w")
        tk.Label(f_esp, text="🔴 SUMO", font=("Arial", 9, "bold"), fg="red").grid(row=0, column=1)
        tk.Label(f_esp, text="🔵 TIRABOLOS", font=("Arial", 9, "bold"), fg="blue").grid(row=0, column=2)

        self.param_especificos = [
            ("IN_RETRO", "Inicio Retroceso (1/0)", "1", "0"),
            ("D_DETEC", "Dist. Detección (cm)", "100", "200"),
            ("T_IN_AV", "T. Inicio Avance (ms)", "800", "1680"),
            ("T_RET", "T. Retroceso (ms)", "700", "1470"),
            ("V_BUSCA", "Velocidad Búsqueda", "180", "120"),
            ("T_GIRO_B", "T. Giro Búsqueda (ms)", "20", "40"),
            ("P_PRE_M", "Pausa Pre-Medir (ms)", "15", "30"),
            ("US_TIME", "Timeout Ultrasónico", "6000", "15000")
        ]

        self.cajas_esp = {}
        row = 1
        for clave, titulo, def_sumo, def_bolo in self.param_especificos:
            tk.Label(f_esp, text=titulo, font=("Arial", 9)).grid(row=row, column=0, sticky="w", pady=4)
            ent_s = tk.Entry(f_esp, width=8, justify="center")
            ent_s.insert(0, def_sumo)
            ent_s.grid(row=row, column=1, padx=5)
            
            ent_b = tk.Entry(f_esp, width=8, justify="center")
            ent_b.insert(0, def_bolo)
            ent_b.grid(row=row, column=2, padx=5)
            
            tk.Button(f_esp, text="Enviar", command=lambda c=clave: self.enviar_especifico(c)).grid(row=row, column=3, padx=5)
            self.cajas_esp[clave] = {'sumo': ent_s, 'bolo': ent_b}
            row += 1

        # Botón maestro
        tk.Button(f_esp, text="🚀 ENVIAR TODOS LOS PARÁMETROS", font=("Arial", 10, "bold"), bg="lightblue", command=self.enviar_todos).grid(row=row, column=0, columnspan=4, pady=15)

        # ---------------------------------------------------------
        # ZONA 3: CONTROLES MANUALES (MANTENER PULSADO)
        # ---------------------------------------------------------
        f_ctrl = tk.LabelFrame(root, text="🕹️ CONDUCCIÓN MANUAL (Requiere Robot Parado)", font=("Arial", 11, "bold"), padx=10, pady=5)
        f_ctrl.pack(fill="x", padx=10, pady=5)

        f_cruceta = tk.Frame(f_ctrl)
        f_cruceta.pack()

        btn_fw = tk.Button(f_cruceta, text="⬆ ADELANTE", width=15, height=2, bg="#ddd")
        btn_fw.grid(row=0, column=1, pady=2)
        btn_fw.bind("<ButtonPress-1>", lambda e: self.enviar_comando(2))
        btn_fw.bind("<ButtonRelease-1>", lambda e: self.enviar_comando(0))

        btn_left = tk.Button(f_cruceta, text="↺ GIRO IZQ", width=15, height=2, bg="#ddd")
        btn_left.grid(row=1, column=0, padx=2)
        btn_left.bind("<ButtonPress-1>", lambda e: self.enviar_comando(4))
        btn_left.bind("<ButtonRelease-1>", lambda e: self.enviar_comando(0))

        btn_stop = tk.Button(f_cruceta, text="FRENAR", width=15, height=2, bg="#555", fg="white", font=("Arial", 9, "bold"))
        btn_stop.grid(row=1, column=1, padx=2)
        btn_stop.bind("<ButtonPress-1>", lambda e: self.enviar_comando(0))

        btn_right = tk.Button(f_cruceta, text="↻ GIRO DER", width=15, height=2, bg="#ddd")
        btn_right.grid(row=1, column=2, padx=2)
        btn_right.bind("<ButtonPress-1>", lambda e: self.enviar_comando(5))
        btn_right.bind("<ButtonRelease-1>", lambda e: self.enviar_comando(0))

        btn_bw = tk.Button(f_cruceta, text="⬇ ATRÁS", width=15, height=2, bg="#ddd")
        btn_bw.grid(row=2, column=1, pady=2)
        btn_bw.bind("<ButtonPress-1>", lambda e: self.enviar_comando(3))
        btn_bw.bind("<ButtonRelease-1>", lambda e: self.enviar_comando(0))

    # --- FUNCIONES DE RED ---
    def enviar_red(self, comando):
        try:
            self.sock.sendto(comando.encode('utf-8'), (self.ip, self.port))
            print(f"Red: {comando.strip()}")
        except Exception as e:
            print(f"Error: {e}")

    def enviar_comando(self, numero):
        self.enviar_red(f"CMD,{numero}\n")

    def enviar_modo(self):
        m = self.modo_var.get()
        self.enviar_red(f"MODO,{m}\n")

    def enviar_compartido(self, clave):
        # Envía el mismo valor al perfil SUMO (1) y BOLO (0) para que el Arduino lo actualice en ambos
        v = self.cajas_comp[clave].get()
        self.enviar_red(f"{clave},1,{v}\n")
        time.sleep(0.01)
        self.enviar_red(f"{clave},0,{v}\n")

    def enviar_especifico(self, clave):
        # Envía el valor de Sumo al índice 1, y el de Bolo al índice 0
        v_sumo = self.cajas_esp[clave]['sumo'].get()
        v_bolo = self.cajas_esp[clave]['bolo'].get()
        self.enviar_red(f"{clave},1,{v_sumo}\n")
        time.sleep(0.01)
        self.enviar_red(f"{clave},0,{v_bolo}\n")

    def enviar_todos(self):
        for clave in self.cajas_comp:
            self.enviar_compartido(clave)
            time.sleep(0.02)
        for clave in self.cajas_esp:
            self.enviar_especifico(clave)
            time.sleep(0.02)
        messagebox.showinfo("Éxito", "¡Todos los parámetros enviados al robot!")

if __name__ == "__main__":
    root = tk.Tk()
    app = GCS_Robot(root)
    root.mainloop()