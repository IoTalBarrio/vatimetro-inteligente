/**
 * Vatimetro IoT al barrio
 * IoTalBarrio@gmail.com
 */

//----------------------------
#include <WiFi.h>
#include <HTTPClient.h>
#include "variables.h"


// Conexión WIFI
const char *ssid = SSID;
const char *password = PASSWORD;

// Web app ID del despliegue webyGoogle Apps Script del desplie
String GOOGLE_SCRIPT_ID = DEP_KEY;

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

/**
 * 
*/
void setup()
{

  // Inicia la comunicacion serial
  Serial.begin(115200);

  // Led de depuración
  pinMode(LED_BUILTIN, OUTPUT);
  //pinMode(PIN_R_V, INPUT);

  Serial.println("OK");
  encenderLED(5, 100);
}

void loop()
{
  // Ralizar el calculo de potencia con los sensores
  Console_Power();
}

void Console_Power()
{
  Serial.println(analogRead(PIN_R_V));

  delay(100);
}

void encenderLED(int veces, int retardo)
{
  for (int i = 0; i < veces; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH); // Enciende el LED_BUILTIN
    delay(retardo);                  // Espera el retardo especificado
    digitalWrite(LED_BUILTIN, LOW);  // Apaga el LED_BUILTIN
    delay(retardo);                  // Espera el retardo especificado
  }
}