// Version 3.8.10 (Dual Mode + Freno Activo + Ultrasonido Trasero + Arranque por Mando)
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
const int TIEMPO_CONTRAMARCHA_RECTO = 70;  // Freno lineal para frente y maniobra de inicio
const int TIEMPO_CONTRAMARCHA_GIRO  = 20; // Latigazo lateral SOLO para el sensor trasero
const int PAUSA_ESTABILIZACION      = 0; 

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
  int  TIEMPO_INICIO_AVANCE = 800;
  int  TIEMPO_RETROCESO     = 730;
  int  VEL_BUSQUEDA         = 180; 
  int  TIEMPO_GIRO          = 20;  
  int  PAUSA_PRE_MEDIR      = 15;  
  unsigned long US_TIMEOUT  = 6000UL; 
#else
  // --- Perfil TIRABOLOS ---
  bool INICIO_RETROCESO     = false; 
  int  DIST_DETECCION       = 200;
  int  TIEMPO_INICIO_AVANCE = 1600;
  int  TIEMPO_RETROCESO     = 1460;
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
    Serial.println(F("===