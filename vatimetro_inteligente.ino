/**
 * Vatimetro IoT al barrio
 * IoTalBarrio@gmail.com
 */

//----------------------------
#include <ArduinoJson.h>
#include "variables.h"

// Web app ID del despliegue webyGoogle Apps Script del desplie
const int SampleCount = 500;  // At 8900 samples per second, about 2 cycles of 60 Hz.
float Samples[SampleCount];
// Pines
/*
(EN) (VP) (VN)
 ¿?   36   39   34   35   32   33
      |    |    |    |    |    |
      YV   BV   RV   RA   BA   RA
*/
#define PIN_Y_V 36
#define PIN_Y_A 35
#define PIN_B_V 39
#define PIN_B_A 32
#define PIN_R_V 34
#define PIN_R_A 33

// Data
const int DELAY_MUESTREO = 150;
const int ARRAY_SIZE = 10;
int voltaje[ARRAY_SIZE];
int corriente[ARRAY_SIZE];
String voltaje_string;
String corriente_string;
int punteroRecolector = 0;

/**
 *
 */
void setup() {

  // Inicia la comunicacion serial
  Serial.begin(115200);
  Serial.flush();

  // Led de depuración
  pinMode(LED_BUILTIN, OUTPUT);

  encenderLED(5, 100);
  
  
}

void loop() {
  recolectar();
  if (punteroRecolector >= ARRAY_SIZE) {
    punteroRecolector = 0;
    Serial.println(construirJson());
  }   
  
}

void recolectar() {
  voltaje[punteroRecolector] =  analogReadSim(PIN_Y_V);
  corriente[punteroRecolector] =  analogReadSim(PIN_Y_A);
  punteroRecolector++;
  Serial.println(punteroRecolector);
  delay(DELAY_MUESTREO);
}

String construirJson() {
  voltaje_string = corriente_string = "[";
  for (int i = 0; i < ARRAY_SIZE; i++) {
      voltaje_string += voltaje[i];
      corriente_string += corriente[i];
      if (i < ARRAY_SIZE - 1) {
          voltaje_string += ", ";
          corriente_string += ", ";
      }
  }
  voltaje_string += "]";
  corriente_string += "]";
  return String("{") + String("'voltaje:'") + voltaje_string + String(", ") + String("'corriente':") + corriente_string + String("}");
 
}

int analogReadSim(int pin) {
  return random(4096);
}

void encenderLED(int veces, int retardo) {
  for (int i = 0; i < veces; i++) {
    digitalWrite(LED_BUILTIN, HIGH);  // Enciende el LED_BUILTIN
    delay(retardo);                   // Espera el retardo especificado
    digitalWrite(LED_BUILTIN, LOW);   // Apaga el LED_BUILTIN
    delay(retardo);                   // Espera el retardo especificado
  }
}