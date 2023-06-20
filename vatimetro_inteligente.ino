/**
 * Vatimetro IoT al barrio
 * IoTalBarrio@gmail.com
 */

//----------------------------
#include <HTTPSRedirect.h>
#include <ArduinoJson.h>
#include "variables.h"

// Web app ID del despliegue webyGoogle Apps Script del desplie
const String hoja = "test";
const char *server = "script.google.com";
const int httpsPort = 443;
const char *fingerprint = "";
String url = String("/macros/s/") + String(DEP_KEY) + "/exec?hoja=" + hoja;
HTTPSRedirect *client = nullptr;
String reponseBody = "";

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

  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");

  Serial.print("Conectando a ");
  Serial.println(server);

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i = 0; i < 5; i++) {
    int retval = client->connect(server, httpsPort);
    if (retval == 1) {
      flag = true;
      Serial.println("¡Conectado!");
      break;
    }
  }
  if (!flag) {
    Serial.print("No pudimos conectarnos a: ");
    Serial.println(server);
  }

  client->POST(url, server, payload, false);
  Serial.println(client->getResponseBody());      
  Serial.println(client->getStatusCode());
  

  delete client;     // delete HTTPSRedirect object
  client = nullptr;  // delete HTTPSRedirect object
  delay(7000);
}

void encenderLED(int veces, int retardo) {
  for (int i = 0; i < veces; i++) {
    digitalWrite(LED_BUILTIN, HIGH);  // Enciende el LED_BUILTIN
    delay(retardo);                   // Espera el retardo especificado
    digitalWrite(LED_BUILTIN, LOW);   // Apaga el LED_BUILTIN
    delay(retardo);                   // Espera el retardo especificado
  }
}