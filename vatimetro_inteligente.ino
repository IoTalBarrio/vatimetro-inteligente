/**
 * Vatimetro IoT al barrio
 * IoTalBarrio@gmail.com
 */

//----------------------------
#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "variables.h"

// Web app ID del despliegue webyGoogle Apps Script del desplie
const String hoja = "test";
const char *server = "script.google.com";
const char *fingerprint = "";
String url = "https://" + String(server) + String("/macros/s/") + String(DEP_KEY) + "/exec?hoja=" + hoja;
HTTPClient http;
int HTTPcode;
String reponseBody;
String locationRedirect;


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
const int ARRAY_SIZE = 5;
int testArray[ARRAY_SIZE];

/**
 *
 */
void setup() {

  // Inicia la comunicacion serial
  Serial.begin(115200);
  Serial.flush();

  // Led de depuración
  pinMode(LED_BUILTIN, OUTPUT);

  // Inicia la comunicacion WiFi
  Serial.print("Conectando a Wi-Fi " + String(SSID));
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    encenderLED(1, 1000);
  }
  Serial.print("¡Conectado a " + String(SSID) + "! ");
  Serial.println(WiFi.localIP());
  encenderLED(5, 100);
}

void loop() {

  // Ralizar el calculo de potencia con los sensores
  // Console_Power();

  Enviar_datos();
}

void Depurar_medicion() {
  Serial.println(analogRead(PIN_R_V));

  delay(100);
}

void Enviar_datos() {
  Serial.println("Enviando datos...");

  String param;
  for (int i = 0; i < ARRAY_SIZE; i++) {
    testArray[i] = i * 10;
  }

  // Crear documento JSON
  const size_t capacity = JSON_ARRAY_SIZE(ARRAY_SIZE);
  StaticJsonDocument<capacity> doc;

  JsonArray voltaje = doc.createNestedArray("voltaje");
  JsonArray corriente = doc.createNestedArray("corriente");

  // Migrar datos de array a Json array
  for (size_t i = 0; i < ARRAY_SIZE; i++) {
    voltaje.add(testArray[i]);
  }
  
  for (size_t i = 0; i < ARRAY_SIZE; i++) {
    corriente.add(testArray[i]);
  }  

  // convertir a cadena del JSON
  String jsonString;
  serializeJson(doc, jsonString);

  Enviar_GoogleAppsScript(jsonString);
}

void Enviar_GoogleAppsScript(String payload) {
  Serial.println(payload);
  http.begin(url);  
  http.addHeader("Content-Type", "application/json"); // Indicamos que vasmo a entregar un JSON        
  // Necesitamos agarrar los headers
  const char *headerKeys[] = {"location"};
  const size_t headerKeysCount = sizeof(headerKeys) / sizeof(headerKeys[0]);
  http.collectHeaders(headerKeys, headerKeysCount);  
  // Hacemos la peticion POST y obtenemos el codigo HTTP de respuesta
  HTTPcode = http.POST(payload);
  Serial.println("HTTP code:" + String(HTTPcode));
  if (HTTPcode == 302) { // Si hay redirección (302)...
    locationRedirect = http.header("location"); // Cogemos la url de redireccion
    Serial.println("Redirección HTTP:" + locationRedirect);
    // Cerramios la coneción anterior y abrimos otra
    http.end();
    http.begin(locationRedirect);  
    HTTPcode = http.GET(); // Y hacemos una nueva petición peroo con GET
    Serial.println("HTTP code (2):" + String(HTTPcode));
    reponseBody = http.getString();
    Serial.println("reponseBody: "+reponseBody);  
  } else {
    http.end();
  }    
  //     
}

void encenderLED(int veces, int retardo) {
  for (int i = 0; i < veces; i++) {
    digitalWrite(LED_BUILTIN, HIGH);  // Enciende el LED_BUILTIN
    delay(retardo);                   // Espera el retardo especificado
    digitalWrite(LED_BUILTIN, LOW);   // Apaga el LED_BUILTIN
    delay(retardo);                   // Espera el retardo especificado
  }
}