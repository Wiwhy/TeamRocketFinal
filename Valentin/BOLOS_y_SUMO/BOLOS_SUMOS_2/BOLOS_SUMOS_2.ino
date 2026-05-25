// Version 3.8.5 (Dual Mode + Freno Activo + Ultrasonido Trasero + Contramarchas Ajustadas)
// ============================================================
//  TIRABOLOS / SUMO - Robot Multi-Pista
//  Ataque Bidireccional (Frente y Espalda)
// ============================================================

// ************************************************************
//  SECCION 1: PANEL DE CONTROL
// ************************************************************

// 1. ELIGE EL MODO DE JUEGO (true = SUMO, false = TIRABOLOS)
#define MODO_SUMO true 

// 2. CALIBRACIÓN DE FRENOS Y PAUSAS
const int TIEMPO_CONTRAMARCHA_RECTO = 70; 
const int TIEMPO_CONTRAMARCHA_GIRO  = 5000; 
const int PAUSA_ESTABILIZACION      = 0; // ms de quietud total para evitar tirones

// 3. CONFIGURACIONES COMPARTIDAS
const int VEL_INICIO_RETRO     = 200;  
const int VEL_INICIO_AVANCE    = 200; 
const int VEL_ATAQUE           = 200; 
const int VEL_RETROCESO        = 200; 
const unsigned long TIMEOUT_BUSQUEDA = 5000UL; 
const int DELAY_PRE_ATAQUE           = 10;      
const int DELAY_POST_RETRO           = 200;    

// 4. CONFIGURACIONES ESPECÍFICAS
#if MODO_SUMO == true
  // --- Perfil SUMO ---
  bool INICIO_RETROCESO     = true; 
  int  DIST_DETECCION       = 100;
  int  TIEMPO_INICIO_AVANCE = 830;
  int  TIEMPO_RETROCESO     = 830;
  int  VEL_BUSQUEDA         = 180; 
  int  TIEMPO_GIRO          = 20;  
  int  PAUSA_PRE_MEDIR      = 15;  
  unsigned long US_TIMEOUT  = 6000UL; 
#else
  // --- Perfil TIRABOLOS ---
  bool INICIO_RETROCESO     = false; 
  int  DIST_DETECCION       = 200;
  int  TIEMPO_INICIO_AVANCE = 1680;
  int  TIEMPO_RETROCESO     = 1470;
  int  VEL_BUSQUEDA         = 120; 
  int  TIEMPO_GIRO          = 40;  
  int  PAUSA_PRE_MEDIR      = 30;  
  unsigned long US_TIMEOUT  = 15000UL; 
#endif

// ************************************************************
//  SECCION 2: PINES 
// ************************************************************

#define speedPinR          9
#define RightMotorDirPin1  22
#define RightMotorDirPin2  24
#define LeftMotorDirPin1   26
#define LeftMotorDirPin2   28
#define speedPinL          10

#define speedPinRB         11
#define RightMotorDirPin1B  5
#define RightMotorDirPin2B  6
#define LeftMotorDirPin1B   7
#define LeftMotorDirPin2B   8
#define speedPinLB         12

// Sensor Frontal
#define TRIG  30
#define ECHO  31

// Sensor Trasero
#define ECHO_T 42
#define TRIG_T 43

#define S1  A4
#define S2  A3
#define S3  A2
#define S4  A1
#define S5  A0

#define ST1  A8
#define ST2  A9
#define ST3  A10
#define ST4  A11
#define ST5  A12

// ************************************************************
//  SECCION 3: ESTADOS Y VARIABLES GLOBALES
// ************************************************************

enum Estado { BUSCAR, ATACAR_FRONTAL, ATACAR_TRASERO, ESCAPAR_ATRAS, ESCAPAR_ADELANTE };
Estado        estado      = BUSCAR;
unsigned long tRetroIni   = 0;       
unsigned long tBuscarIni  = 0;       

// ************************************************************
//  SECCION 4: SETUP
// ************************************************************

void setup() {
  Serial.begin(9600);

  #if MODO_SUMO == true
    Serial.println(F("=== MODO: SUMO ==="));
  #else
    Serial.println(F("=== MODO: TIRABOLOS ==="));
  #endif

  pinMode(RightMotorDirPin1,  OUTPUT); pinMode(RightMotorDirPin2,  OUTPUT);
  pinMode(LeftMotorDirPin1,   OUTPUT); pinMode(LeftMotorDirPin2,   OUTPUT);
  pinMode(speedPinR,          OUTPUT); pinMode(speedPinL,          OUTPUT);
  pinMode(RightMotorDirPin1B, OUTPUT); pinMode(RightMotorDirPin2B, OUTPUT);
  pinMode(LeftMotorDirPin1B,  OUTPUT); pinMode(LeftMotorDirPin2B,  OUTPUT);
  pinMode(speedPinRB,         OUTPUT); pinMode(speedPinLB,         OUTPUT);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  digitalWrite(TRIG, LOW);

  pinMode(TRIG_T, OUTPUT);
  pinMode(ECHO_T, INPUT);
  digitalWrite(TRIG_T, LOW);

  pinMode(S1, INPUT); pinMode(S2, INPUT); pinMode(S3, INPUT);
  pinMode(S4, INPUT); pinMode(S5, INPUT);
  pinMode(ST1, INPUT); pinMode(ST2, INPUT); pinMode(ST3, INPUT);
  pinMode(ST4, INPUT); pinMode(ST5, INPUT);

  parar();

  if (INICIO_RETROCESO) {
    maniobraInicio();
  }

  tBuscarIni = millis();
}

// ************************************************************
//  SECCION 5: MANIOBRA DE INICIO
// ************************************************************

void maniobraInicio() {
  retroceder(); 
  velocidad(VEL_INICIO_RETRO);
  while (!hayLineaTrasera()) { delay(5); }

  avanzar();
  velocidad(255);
  delay(15);         
  frenoMagnetico();
  delay(15);         
  parar();
  delay(PAUSA_ESTABILIZACION);

  avanzar();
  velocidad(VEL_INICIO_AVANCE);
  delay(TIEMPO_INICIO_AVANCE); 

  retroceder();
  velocidad(255);
  delay(TIEMPO_CONTRAMARCHA_RECTO);
  frenoMagnetico();
  delay(50);
  parar();
  delay(PAUSA_ESTABILIZACION);  
}

// ************************************************************
//  SECCION 6: LOOP PRINCIPAL
// ************************************************************

void loop() {
  bool lineaFrente = hayLinea();
  bool lineaAtras  = hayLineaTrasera();

  switch (estado) {

    // --------------------------------------------------------
    //  ESTADO: BUSCAR
    // --------------------------------------------------------
    case BUSCAR:
      if (lineaFrente) {
        girar(); // Contramarcha hacia la derecha
        velocidad(255);
        delay(TIEMPO_CONTRAMARCHA_GIRO); 
        frenoMagnetico();
        delay(50);
        parar();
        delay(PAUSA_ESTABILIZACION);
        
        estado    = ESCAPAR_ATRAS;
        tRetroIni = millis();
        break;
      }
      
      if (lineaAtras) {
        girarInverso(); // <-- Contramarcha hacia la izquierda
        velocidad(255);
        delay(TIEMPO_CONTRAMARCHA_GIRO); 
        frenoMagnetico();
        delay(50);
        parar();
        delay(PAUSA_ESTABILIZACION);
        
        estado    = ESCAPAR_ADELANTE;
        tRetroIni = millis();
        break;
      }

      if (millis() - tBuscarIni >= TIMEOUT_BUSQUEDA) {
        estado = ATACAR_FRONTAL;
        break;
      }

      girar();
      velocidad(VEL_BUSQUEDA);
      delay(TIEMPO_GIRO);

      {
        frenoMagnetico(); 
        delay(15);
        parar();
        delay(PAUSA_PRE_MEDIR);
        
        // 1. Medir Frente
        int dFrente = medirMediana(TRIG, ECHO);
        if (dFrente > 0 && dFrente <= DIST_DETECCION) {
          delay(DELAY_PRE_ATAQUE);
          estado = ATACAR_FRONTAL;
          break; // Rompe para no medir atrás si ya vio algo al frente
        }

        // 2. Medir Atrás
        int dAtras = medirMediana(TRIG_T, ECHO_T);
        if (dAtras > 0 && dAtras <= DIST_DETECCION) {
          delay(DELAY_PRE_ATAQUE);
          estado = ATACAR_TRASERO;
        }
      }
      break;

    // --------------------------------------------------------
    //  ESTADO: ATACAR FRONTAL
    // --------------------------------------------------------
    case ATACAR_FRONTAL:
      if (lineaFrente) {
        retroceder();
        velocidad(255);
        delay(TIEMPO_CONTRAMARCHA_RECTO); 
        frenoMagnetico();
        delay(50);
        parar();
        delay(PAUSA_ESTABILIZACION);
        
        estado    = ESCAPAR_ATRAS;
        tRetroIni = millis();
        break;
      }

      avanzar();
      velocidad(VEL_ATAQUE);
      break;

    // --------------------------------------------------------
    //  ESTADO: ATACAR TRASERO
    // --------------------------------------------------------
    case ATACAR_TRASERO:
      if (lineaAtras) {
        avanzar(); // Freno en seco yendo hacia atrás
        velocidad(255);
        delay(TIEMPO_CONTRAMARCHA_RECTO); 
        frenoMagnetico();
        delay(50);
        parar();
        delay(PAUSA_ESTABILIZACION);
        
        estado    = ESCAPAR_ADELANTE;
        tRetroIni = millis();
        break;
      }

      retroceder();
      velocidad(VEL_ATAQUE);
      break;

    // --------------------------------------------------------
    //  ESTADO: ESCAPAR_ATRAS (Tocó línea frontal)
    // --------------------------------------------------------
    case ESCAPAR_ATRAS:
      if (millis() - tRetroIni < (unsigned long)TIEMPO_RETROCESO) {
        retroceder();
        velocidad(VEL_RETROCESO);
      } else {
        avanzar();
        velocidad(255);
        delay(30);
        frenoMagnetico();
        delay(50);
        parar();
        delay(PAUSA_ESTABILIZACION); 
        delay(DELAY_POST_RETRO); 
        
        medirMediana(TRIG, ECHO);     // Limpia buffer frontal
        medirMediana(TRIG_T, ECHO_T); // Limpia buffer trasero

        estado = BUSCAR;
        tBuscarIni = millis();  
      }
      break;

    // --------------------------------------------------------
    //  ESTADO: ESCAPAR_ADELANTE (Tocó línea trasera)
    // --------------------------------------------------------
    case ESCAPAR_ADELANTE:
      if (millis() - tRetroIni < (unsigned long)TIEMPO_RETROCESO) {
        avanzar();
        velocidad(VEL_RETROCESO);
      } else {
        retroceder();
        velocidad(255);
        delay(30);
        frenoMagnetico();
        delay(50);
        parar();
        delay(PAUSA_ESTABILIZACION); 
        delay(DELAY_POST_RETRO); 
        
        medirMediana(TRIG, ECHO);     // Limpia buffer frontal
        medirMediana(TRIG_T, ECHO_T); // Limpia buffer trasero

        estado = BUSCAR;
        tBuscarIni = millis();  
      }
      break;
  }
}

// ************************************************************
//  SECCION 7: ULTRASONIDO
// ************************************************************

int medir(int pinTrig, int pinEcho) {
  digitalWrite(pinTrig, LOW);
  delayMicroseconds(4);
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig, LOW);
  
  long us = pulseIn(pinEcho, HIGH, US_TIMEOUT); 
  
  if (us == 0) return 999;        
  int cm = (int)(us / 58);
  if (cm < 2) return 999;          
  return cm;
}

int medirMediana(int pinTrig, int pinEcho) {
  int a = medir(pinTrig, pinEcho); delayMicroseconds(500);
  int b = medir(pinTrig, pinEcho); delayMicroseconds(500);
  int c = medir(pinTrig, pinEcho);
  
  if (a > b) { int t = a; a = b; b = t; }
  if (b > c) { int t = b; b = c; c = t; }
  if (a > b) { int t = a; a = b; b = t; }
  
  return b;  
}

// ************************************************************
//  SECCION 8: SENSORES DE LINEA
// ************************************************************

bool hayLinea() {
  return (!digitalRead(S1) || !digitalRead(S2) || !digitalRead(S3) ||
          !digitalRead(S4) || !digitalRead(S5));
}

bool hayLineaTrasera() {
  return (!digitalRead(ST1) || !digitalRead(ST2) || !digitalRead(ST3) ||
          !digitalRead(ST4) || !digitalRead(ST5));
}

// ************************************************************
//  SECCION 9: MOVIMIENTO Y FRENOS L298N
// ************************************************************

void velocidad(int v) {
  analogWrite(speedPinL,  v); analogWrite(speedPinR,  v);
  analogWrite(speedPinLB, v); analogWrite(speedPinRB, v);
}

void avanzar()      { FRf(); FLf(); RRf(); RLf(); }  
void retroceder()   { FRb(); FLb(); RRb(); RLb(); }  
void girar()        { FRb(); FLf(); RRb(); RLf(); }  
void girarInverso() { FRf(); FLb(); RRf(); RLb(); } 

void frenoMagnetico() {
  digitalWrite(RightMotorDirPin1,  LOW); digitalWrite(RightMotorDirPin2,  LOW);
  digitalWrite(LeftMotorDirPin1,   LOW); digitalWrite(LeftMotorDirPin2,   LOW);
  digitalWrite(RightMotorDirPin1B, LOW); digitalWrite(RightMotorDirPin2B, LOW);
  digitalWrite(LeftMotorDirPin1B,  LOW); digitalWrite(LeftMotorDirPin2B,  LOW);
  velocidad(255); 
}

void parar() {
  digitalWrite(RightMotorDirPin1,  LOW); digitalWrite(RightMotorDirPin2,  LOW);
  digitalWrite(LeftMotorDirPin1,   LOW); digitalWrite(LeftMotorDirPin2,   LOW);
  digitalWrite(RightMotorDirPin1B, LOW); digitalWrite(RightMotorDirPin2B, LOW);
  digitalWrite(LeftMotorDirPin1B,  LOW); digitalWrite(LeftMotorDirPin2B,  LOW);
  velocidad(0); 
}

void FRf() { digitalWrite(RightMotorDirPin1,  LOW);  digitalWrite(RightMotorDirPin2,  HIGH); }
void FRb() { digitalWrite(RightMotorDirPin1,  HIGH); digitalWrite(RightMotorDirPin2,  LOW);  }
void FLf() { digitalWrite(LeftMotorDirPin1,   LOW);  digitalWrite(LeftMotorDirPin2,   HIGH); }
void FLb() { digitalWrite(LeftMotorDirPin1,   HIGH); digitalWrite(LeftMotorDirPin2,   LOW);  }
void RRf() { digitalWrite(RightMotorDirPin1B, LOW);  digitalWrite(RightMotorDirPin2B, HIGH); }
void RRb() { digitalWrite(RightMotorDirPin1B, HIGH); digitalWrite(RightMotorDirPin2B, LOW);  }
void RLf() { digitalWrite(LeftMotorDirPin1B,  LOW);  digitalWrite(LeftMotorDirPin2B,  HIGH); }
void RLb() { digitalWrite(LeftMotorDirPin1B,  HIGH); digitalWrite(LeftMotorDirPin2B,  LOW);  }
