/**
 * Vatimetro IoT al barrio
 * IoTalBarrio@gmail.com
 */

//----------------------------
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "variables.h"

// Conexión WIFI
const char *ssid = SSID;
const char *password = PASSWORD;

// Web app ID del despliegue webyGoogle Apps Script del desplie
const String GOOGLE_SCRIPT_ID = DEP_KEY;
const String HOJA = "test";

const int SampleCount = 500; // At 8900 samples per second, about 2 cycles of 60 Hz.
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
const int ARRAY_SIZE = 5;
int testArray[ARRAY_SIZE]; 


/**
 *
 */
void setup()
{

  // Inicia la comunicacion serial
  Serial.begin(115200);

  // Led de depuración
  pinMode(LED_BUILTIN, OUTPUT);

  // Inicia la comunicacion WiFi
  Serial.print("Conectando a Wi-Fi "+String(SSID));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    encenderLED(1, 1000);
  }
  Serial.println("¡Conectado a "+String(SSID)+"!");
  encenderLED(5, 100);
}

void loop()
{
  // Ralizar el calculo de potencia con los sensores
  //Console_Power();
  
  //  
  Enviar_datos();
}

void Depurar_medicion()
{
  Serial.println(analogRead(PIN_R_V));

  delay(100);
}

void Enviar_datos()
{
  // Primer par
  String param;
  for (int i = 0; i < ARRAY_SIZE; i++) {
    testArray[i] = i * 10;
  }

  // Crear documento JSON
  const size_t capacity = JSON_ARRAY_SIZE(ARRAY_SIZE);
  StaticJsonDocument<capacity> doc;  

  // Crear array JSON 
  JsonArray jsonArray = doc.to<JsonArray>();

  // Migrar datos de array a Json array
  for (size_t i = 0; i < ARRAY_SIZE; i++) {
    jsonArray.add(testArray[i]);
  }

  // convertir a cadena del JSON
  String jsonString;
  serializeJson(doc, jsonString);  

  
  Enviar_GoogleAppsScript(jsonString);
}

void Enviar_GoogleAppsScript(String body)
{
  HTTPClient http;
  String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?hoja="+HOJA;
  Serial.println("Subiendo datos a google sheets: " + url);
  Serial.println(body);

  // starts posting data to google sheet
  http.begin(url.c_str());
  // Le decimos al servidor que el cuerpo de la petición es en JSON
  http.addHeader("Content-Type", "application/json");

  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.POST(body);
  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);

  // Respuesta de Google Apps Scripts Web
  String payload;
  payload = http.getString();
  Serial.println("Cuerpo de respuesta HTTP: " + payload);  

  if (httpCode > 0)
  {
    encenderLED(3, 100);
  }
  else
  {
    encenderLED(2, 500);
  }
  http.end();
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