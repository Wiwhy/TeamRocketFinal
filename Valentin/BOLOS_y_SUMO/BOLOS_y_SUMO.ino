// Version 3.8 (Dual Mode + Freno Activo por Contramarcha AUMENTADO)
// ============================================================
//  TIRABOLOS / SUMO - Robot Multi-Pista
//  BUSCAR -> ATACAR -> RETROCEDER (loop infinito)
// ============================================================

// ************************************************************
//  SECCION 1: MODO DE COMPETICION Y CONFIGURACION
// ************************************************************

// Cambia esta variable a true o false según lo que vayas a jugar:
bool MODO_SUMO = true; 

// 🚨 CALIBRACIÓN DEL FRENO POR CONTRAMARCHA (TIEMPOS AUMENTADOS) 🚨
const int TIEMPO_CONTRAMARCHA_RECTO = 70; // Subido a 70ms (Frena la masa a máxima velocidad)
const int TIEMPO_CONTRAMARCHA_GIRO  = 50; // Subido a 50ms (Clava el robot al girar)

// --- Variables dependientes del modo ---
bool INICIO_RETROCESO;
int  DIST_DETECCION;
int  TIEMPO_INICIO_AVANCE;
int  TIEMPO_RETROCESO;
int  VEL_BUSQUEDA;
int  TIEMPO_GIRO;
int  PAUSA_PRE_MEDIR;
unsigned long US_TIMEOUT;

// --- Configuraciones Generales Compartidas ---
const int VEL_INICIO_RETRO     = 150;  
const int VEL_INICIO_AVANCE    = 200; 
const int VEL_ATAQUE           = 200; 
const int VEL_RETROCESO        = 190; 

const unsigned long TIMEOUT_BUSQUEDA = 3500UL; 
const int DELAY_PRE_ATAQUE           = 0;      
const int DELAY_POST_RETRO           = 200;    

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

#define TRIG  30
#define ECHO  31

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

enum Estado { BUSCAR, ATACAR, RETROCEDER };
Estado        estado      = BUSCAR;
unsigned long tRetroIni   = 0;       
unsigned long tBuscarIni  = 0;       

// ************************************************************
//  SECCION 4: SETUP
// ************************************************************

void setup() {
  Serial.begin(9600);

  if (MODO_SUMO == true) {
    INICIO_RETROCESO     = true; 
    DIST_DETECCION       = 100;
    TIEMPO_INICIO_AVANCE = 700;
    TIEMPO_RETROCESO     = 700;
    VEL_BUSQUEDA         = 180; 
    TIEMPO_GIRO          = 20;  
    PAUSA_PRE_MEDIR      = 15;  
    US_TIMEOUT           = 6000UL; 
  } else {
    INICIO_RETROCESO     = false; 
    DIST_DETECCION       = 200;
    TIEMPO_INICIO_AVANCE = 1400;
    TIEMPO_RETROCESO     = 1400;
    VEL_BUSQUEDA         = 140; 
    TIEMPO_GIRO          = 45;  
    PAUSA_PRE_MEDIR      = 30;  
    US_TIMEOUT           = 15000UL; 
  }

  pinMode(RightMotorDirPin1,  OUTPUT); pinMode(RightMotorDirPin2,  OUTPUT);
  pinMode(LeftMotorDirPin1,   OUTPUT); pinMode(LeftMotorDirPin2,   OUTPUT);
  pinMode(speedPinR,          OUTPUT); pinMode(speedPinL,          OUTPUT);
  pinMode(RightMotorDirPin1B, OUTPUT); pinMode(RightMotorDirPin2B, OUTPUT);
  pinMode(LeftMotorDirPin1B,  OUTPUT); pinMode(LeftMotorDirPin2B,  OUTPUT);
  pinMode(speedPinRB,         OUTPUT); pinMode(speedPinLB,         OUTPUT);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  digitalWrite(TRIG, LOW);

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
  velocidad(VEL_INICIO_RETRO);
  retroceder();
while (!hayLineaTrasera()) { delay(5); } // <-- El robot retrocede hasta que toca la línea trasera

  velocidad(255);
  avanzar();
  delay(15);         // 1. Minipausa fija (30ms) para frenar con contramarcha
  frenoMagnetico();
  delay(15);         // 2. Minipausa fija (50ms) mientras aplica el freno magnético
  parar();
       // 3. PAUSA FIJA DE ESPERA (100ms) con motores apagados antes de salir

  velocidad(VEL_INICIO_AVANCE);
  avanzar();
  delay(TIEMPO_INICIO_AVANCE); // 4. VARIABLE que dicta cuánto tiempo avanza hacia adelante

  velocidad(255);
  retroceder();
  delay(TIEMPO_CONTRAMARCHA_RECTO);
  frenoMagnetico();
  delay(50);
  parar();
  delay(200);  
}

// ************************************************************
//  SECCION 6: LOOP PRINCIPAL
// ************************************************************

void loop() {
  bool linea = hayLinea();

  switch (estado) {

    // --------------------------------------------------------
    //  ESTADO: BUSCAR
    // --------------------------------------------------------
    case BUSCAR:
      if (linea) {
        velocidad(255);
        girarInverso(); 
        delay(TIEMPO_CONTRAMARCHA_GIRO); 
        frenoMagnetico();
        delay(50);
        parar();
        
        estado    = RETROCEDER;
        tRetroIni = millis();
        break;
      }

      if (millis() - tBuscarIni >= TIMEOUT_BUSQUEDA) {
        estado = ATACAR;
        break;
      }

      velocidad(VEL_BUSQUEDA);
      girar();
      delay(TIEMPO_GIRO);

      {
        frenoMagnetico(); 
        delay(15);
        parar();
        
        delay(PAUSA_PRE_MEDIR);
        int d = medirMediana();

        if (d > 0 && d <= DIST_DETECCION) {
          delay(DELAY_PRE_ATAQUE);
          estado = ATACAR;
        }
      }
      break;

    // --------------------------------------------------------
    //  ESTADO: ATACAR
    // --------------------------------------------------------
    case ATACAR:
      if (linea) {
        velocidad(255);
        retroceder();
        delay(TIEMPO_CONTRAMARCHA_RECTO); 
        frenoMagnetico();
        delay(50);
        parar();
        
        estado    = RETROCEDER;
        tRetroIni = millis();
        break;
      }

      velocidad(VEL_ATAQUE);
      avanzar();
      break;

    // --------------------------------------------------------
    //  ESTADO: RETROCEDER
    // --------------------------------------------------------
    case RETROCEDER:
      if (millis() - tRetroIni < (unsigned long)TIEMPO_RETROCESO) {
        velocidad(VEL_RETROCESO);
        retroceder();
      } else {
        velocidad(255);
        avanzar();
        delay(30);
        frenoMagnetico();
        delay(50);
        parar();
        
        delay(DELAY_POST_RETRO); 
        medirMediana(); 

        estado = BUSCAR;
        tBuscarIni = millis();  
      }
      break;
  }
}

// ************************************************************
//  SECCION 7: ULTRASONIDO
// ************************************************************

int medir() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(4);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  
  long us = pulseIn(ECHO, HIGH, US_TIMEOUT); 
  
  if (us == 0) return 999;        
  int cm = (int)(us / 58);
  if (cm < 2) return 999;          
  return cm;
}

int medirMediana() {
  int a = medir(); delayMicroseconds(500);
  int b = medir(); delayMicroseconds(500);
  int c = medir();
  
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