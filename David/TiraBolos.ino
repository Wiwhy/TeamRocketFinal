// ==========================================
// CONFIGURACIÓN DE PINES (Hardware)
// ==========================================
#define speedPinR 9
#define RightMotorDirPin1 22
#define RightMotorDirPin2 24
#define LeftMotorDirPin1 26
#define LeftMotorDirPin2 28
#define speedPinL 10

#define speedPinRB 11
#define RightMotorDirPin1B 5
#define RightMotorDirPin2B 6
#define LeftMotorDirPin1B 7
#define LeftMotorDirPin2B 8
#define speedPinLB 12

// Arrays de sensores de línea (leídos como digitales)
// Delanteros: A0, A1, A2, A3, A4
// Traseros:   A8, A9, A10, A11, A12
const int sensoresDelanteros[5] = {A0, A1, A2, A3, A4};
const int sensoresTraseros[5] = {A8, A9, A10, A11, A12};

// ==========================================
// VARIABLES DE CALIBRACIÓN (Ajustar en pista)
// ==========================================
int VELOCIDAD_BARRIDO = 130; // Velocidad de cruce recto transversal (0-255)
int VEL_GIRO_90 = 80;        // Velocidad del giro inicial de 90° (0-255)
int TIEMPO_ATRAS_CENTRO =
    350; // Tiempo (ms) para alejarse del borde en la inicialización
int TIEMPO_GIRO_90 = 700; // Tiempo (ms) para completar el giro inicial de 90°

// --- Control de curva en movimiento (conducción diferencial sin parar) ---
// Al llegar a un borde, el robot NO frena: invierte la marcha y aplica
// velocidades distintas a cada lado durante TIEMPO_GIRO_X ms para angularse.
//
//   VEL_CURVA_EXTERIOR → ruedas del lado exterior de la curva (más rápidas)
//   VEL_CURVA_INTERIOR → ruedas del lado interior de la curva (más lentas)
//
//   ↑ Aumentar diferencia (ext - int) → curva más cerrada → mayor salto lateral
//   por pasada. ↓ Reducir  diferencia             → curva más abierta → pasadas
//   más paralelas.
int VEL_CURVA_EXTERIOR = 160; // Velocidad ruedas exterior (0-255)
int VEL_CURVA_INTERIOR = 50;  // Velocidad ruedas interior (0-255)

// TIEMPO_GIRO_ATRAS:    duración (ms) de la curva al iniciar marcha ATRÁS
// (toque impar) TIEMPO_GIRO_ADELANTE: duración (ms) de la curva al iniciar
// marcha ADELANTE (toque par)
//   ↑ Subir → mayor ángulo acumulado → más área barrida por pasada.
//   ↓ Bajar → pasadas más rectas → menor desplazamiento lateral.
int TIEMPO_GIRO_ATRAS = 150;    // ms de curva al cambiar a marcha ATRÁS
int TIEMPO_GIRO_ADELANTE = 200; // ms de curva al cambiar a marcha ADELANTE

int MAX_TOQUES = 11; // Número de cruces transversales antes de detenerse

// --- Contrafrenada activa (SOLO usada en los pasos de inicialización 1-4b) ---
// No se usa dentro del bucle de zigzag (rebote continuo sin paradas).
//   Si el robot sigue avanzando al parar  → sube TIEMPO o VEL.
//   Si el robot rebota hacia atrás        → baja  TIEMPO o VEL.
int TIEMPO_FRENADA_ADELANTE = 80; // ms del pulso de freno viniendo de adelante
int VEL_FRENADA_ADELANTE = 70;    // velocidad del pulso de freno (adelante)
int TIEMPO_FRENADA_ATRAS = 80;    // ms del pulso de freno viniendo de atrás
int VEL_FRENADA_ATRAS = 70;       // velocidad del pulso de freno (atrás)
int TIEMPO_FRENADA_GIRO = 80;     // ms del pulso de freno en giro
int VEL_FRENADA_GIRO = 70;        // velocidad del pulso de freno (giro)

// ==========================================
// CONTROL BÁSICO DE MOTORES INDIVIDUALES
// ==========================================
void FR_fwd(int s) {
  digitalWrite(RightMotorDirPin1, LOW);
  digitalWrite(RightMotorDirPin2, HIGH);
  analogWrite(speedPinR, s);
}
void FR_bck(int s) {
  digitalWrite(RightMotorDirPin1, HIGH);
  digitalWrite(RightMotorDirPin2, LOW);
  analogWrite(speedPinR, s);
}

void FL_fwd(int s) {
  digitalWrite(LeftMotorDirPin1, LOW);
  digitalWrite(LeftMotorDirPin2, HIGH);
  analogWrite(speedPinL, s);
}
void FL_bck(int s) {
  digitalWrite(LeftMotorDirPin1, HIGH);
  digitalWrite(LeftMotorDirPin2, LOW);
  analogWrite(speedPinL, s);
}

void RR_fwd(int s) {
  digitalWrite(RightMotorDirPin1B, LOW);
  digitalWrite(RightMotorDirPin2B, HIGH);
  analogWrite(speedPinRB, s);
}
void RR_bck(int s) {
  digitalWrite(RightMotorDirPin1B, HIGH);
  digitalWrite(RightMotorDirPin2B, LOW);
  analogWrite(speedPinRB, s);
}

// Motor trasero izquierdo (RL) con corrección del 11% para compensar desvío
// mecánico
void RL_fwd(int s) {
  int c = constrain((int)(s * 1.11), 0, 255);
  digitalWrite(LeftMotorDirPin1B, LOW);
  digitalWrite(LeftMotorDirPin2B, HIGH);
  analogWrite(speedPinLB, c);
}
void RL_bck(int s) {
  int c = constrain((int)(s * 1.11), 0, 255);
  digitalWrite(LeftMotorDirPin1B, HIGH);
  digitalWrite(LeftMotorDirPin2B, LOW);
  analogWrite(speedPinLB, c);
}

// ==========================================
// CINEMÁTICA MECANUM (movimiento base)
// ==========================================
void adelante(int vel) {
  FL_fwd(vel);
  FR_fwd(vel);
  RL_fwd(vel);
  RR_fwd(vel);
}
void atras(int vel) {
  FL_bck(vel);
  FR_bck(vel);
  RL_bck(vel);
  RR_bck(vel);
}
void giroDerecha(int vel) {
  FL_fwd(vel);
  FR_bck(vel);
  RL_fwd(vel);
  RR_bck(vel);
}
void giroIzquierda(int vel) {
  FL_bck(vel);
  FR_fwd(vel);
  RL_bck(vel);
  RR_fwd(vel);
}

void parar() {
  analogWrite(speedPinL, 0);
  analogWrite(speedPinR, 0);
  analogWrite(speedPinLB, 0);
  analogWrite(speedPinRB, 0);
  digitalWrite(RightMotorDirPin1, LOW);
  digitalWrite(RightMotorDirPin2, LOW);
  digitalWrite(LeftMotorDirPin1, LOW);
  digitalWrite(LeftMotorDirPin2, LOW);
  digitalWrite(RightMotorDirPin1B, LOW);
  digitalWrite(RightMotorDirPin2B, LOW);
  digitalWrite(LeftMotorDirPin1B, LOW);
  digitalWrite(LeftMotorDirPin2B, LOW);
}

// ==========================================
// CURVAS EN MOVIMIENTO (conducción diferencial)
// ==========================================
// Sin detenerse: velocidades distintas en cada lado para angularse.
// Las funciones RL_* ya incluyen la corrección del 11%.

// Avanza curvando a la DERECHA (izquierda=exterior, derecha=interior)
void curvaAdelanteDer(int velExt, int velInt) {
  FL_fwd(velExt);
  FR_fwd(velInt);
  RL_fwd(velExt);
  RR_fwd(velInt);
}

// Retrocede curvando a la IZQUIERDA (izquierda=exterior en reversa,
// derecha=interior)
void curvaAtrasIzq(int velExt, int velInt) {
  FL_bck(velExt);
  FR_bck(velInt);
  RL_bck(velExt);
  RR_bck(velInt);
}

// ==========================================
// FRENADA ACTIVA (SOLO para la inicialización, pasos 1-4b)
// ==========================================
void frenarAdelante() {
  atras(VEL_FRENADA_ADELANTE);
  delay(TIEMPO_FRENADA_ADELANTE);
  parar();
  delay(200);
}
void frenarAtras() {
  adelante(VEL_FRENADA_ATRAS);
  delay(TIEMPO_FRENADA_ATRAS);
  parar();
  delay(200);
}
void frenarGiroDer() {
  giroIzquierda(VEL_FRENADA_GIRO);
  delay(TIEMPO_FRENADA_GIRO);
  parar();
  delay(200);
}

// ==========================================
// LECTURA DE SENSORES — OR lógico de 5 sensores
// ==========================================
// true si CUALQUIERA de los 5 sensores delanteros lee LOW (línea negra).
bool detectaLineaDelantera() {
  for (int i = 0; i < 5; i++) {
    if (digitalRead(sensoresDelanteros[i]) == LOW)
      return true;
  }
  return false;
}

// true si CUALQUIERA de los 5 sensores traseros lee LOW (línea negra).
bool detectaLineaTrasera() {
  for (int i = 0; i < 5; i++) {
    if (digitalRead(sensoresTraseros[i]) == LOW)
      return true;
  }
  return false;
}

// ==========================================
// RUTINA PRINCIPAL — ZIGZAG DIAGONAL
// ==========================================
//
// Inicialización (pasos 1-4b): frenada activa para posicionarse con precisión.
// Bucle zigzag   (paso 5):     rebote continuo, el robot NUNCA para entre
// cruces.
//
// Patrón:
//   Toque IMPAR  → borde NORTE (sensor delantero) → curvaAtrasIzq + cruce ATRÁS
//   Toque PAR    → borde SUR   (sensor trasero)   → curvaAdelanteDer + cruce
//   ADELANTE
//
void iniciarBarrido() {

  // ── PASO 1: Avanzar hasta el borde norte ──────────────────────────────
  adelante(VELOCIDAD_BARRIDO);
  while (!detectaLineaDelantera()) {
  }
  frenarAdelante();

  // ── PASO 2: Retroceder para despegarse de la línea ────────────────────
  atras(VELOCIDAD_BARRIDO);
  delay(TIEMPO_ATRAS_CENTRO);
  frenarAtras();

  // ── PASO 3: Giro 90° a la derecha → orientado al borde derecho ────────
  giroDerecha(VEL_GIRO_90);
  delay(TIEMPO_GIRO_90);
  frenarGiroDer();

  // ── PASO 4a: Marcha atrás hasta que el sensor TRASERO toque el borde sur
  atras(VELOCIDAD_BARRIDO);
  while (!detectaLineaTrasera()) {
  }
  frenarAtras();

  // ── PASO 4b: Avanzar recto hasta que el sensor DELANTERO toque el borde
  // norte
  adelante(VELOCIDAD_BARRIDO);
  while (!detectaLineaDelantera()) {
  }
  frenarAdelante();

  // ============================================================
  // PASO 5 — Bucle de zigzag: REBOTE CONTINUO (sin paradas)
  // ============================================================
  // Al detectar la línea NO se para: se invierte la marcha y se aplica
  // curva diferencial (velExt != velInt) para angularse hacia la siguiente
  // franja. Tras TIEMPO_GIRO_X ms de curva, se retoma la marcha recta a
  // VELOCIDAD_BARRIDO.
  // ============================================================

  int toquesActuales =
      1; // El paso 4b ya cuenta como primer toque del borde norte

  while (toquesActuales < MAX_TOQUES) {

    if (toquesActuales % 2 != 0) {
      // ── TOQUE IMPAR: estamos en borde NORTE → cruzar hacia ATRÁS ──────
      // Curva a la izquierda en marcha atrás: desplaza el robot
      // diagonalmente hacia el lado izquierdo del círculo.
      curvaAtrasIzq(VEL_CURVA_EXTERIOR, VEL_CURVA_INTERIOR);
      delay(TIEMPO_GIRO_ATRAS);

      // Cruce recto hasta que el sensor trasero detecte el borde SUR
      atras(VELOCIDAD_BARRIDO);
      while (!detectaLineaTrasera()) {
      }

    } else {
      // ── TOQUE PAR: estamos en borde SUR → cruzar hacia ADELANTE ───────
      // Curva a la derecha en marcha adelante: desplaza el robot
      // diagonalmente hacia el lado derecho del círculo.
      curvaAdelanteDer(VEL_CURVA_EXTERIOR, VEL_CURVA_INTERIOR);
      delay(TIEMPO_GIRO_ADELANTE);

      // Cruce recto hasta que el sensor delantero detecte el borde NORTE
      adelante(VELOCIDAD_BARRIDO);
      while (!detectaLineaDelantera()) {
      }
    }

    toquesActuales++;
  }

  parar(); // Fin del barrido
}

// ==========================================
// SETUP Y LOOP
// ==========================================
void setup() {
  // Motores como salidas
  pinMode(RightMotorDirPin1, OUTPUT);
  pinMode(RightMotorDirPin2, OUTPUT);
  pinMode(LeftMotorDirPin1, OUTPUT);
  pinMode(LeftMotorDirPin2, OUTPUT);
  pinMode(speedPinL, OUTPUT);
  pinMode(speedPinR, OUTPUT);
  pinMode(RightMotorDirPin1B, OUTPUT);
  pinMode(RightMotorDirPin2B, OUTPUT);
  pinMode(LeftMotorDirPin1B, OUTPUT);
  pinMode(LeftMotorDirPin2B, OUTPUT);
  pinMode(speedPinLB, OUTPUT);
  pinMode(speedPinRB, OUTPUT);

  // Sensores como entradas
  for (int i = 0; i < 5; i++) {
    pinMode(sensoresDelanteros[i], INPUT);
    pinMode(sensoresTraseros[i], INPUT);
  }

  parar();
  delay(2000); // Tiempo de seguridad antes de arrancar

  iniciarBarrido();
}

void loop() {
  // El robot ejecuta la rutina una sola vez.
  // Para repetir, presiona el botón RESET del Arduino.
}