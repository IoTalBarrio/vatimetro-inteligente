/**
 * Vatimetro IoT al barrio
 * IoTalBarrio@gmail.com
 */

//----------------------------
#include <./libraries/WiFi/src/WiFi.h>
#include <./libraries/HTTPSRedirect/HTTPSRedirect.h>
#include <ArduinoJson.h>
#include "variables.h"

// Conexión WIFI
const char *ssid = SSID;
const char *password = PASSWORD;

// Web app ID del despliegue webyGoogle Apps Script del desplie
const String GOOGLE_SCRIPT_ID = DEP_KEY;
const String HOJA = "test";
const char* host = "script.google.com";
const int httpsPort = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + String(DEP_KEY) + "/exec?hoja="+HOJA;
HTTPSRedirect* client = nullptr;

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
  Serial.print("¡Conectado a "+String(SSID)+"! ");
  Serial.println(WiFi.localIP());
  encenderLED(5, 100);

  // Inicia la comunicación HTTP (con redirección)
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  
  Serial.print("Conectando a ");
  Serial.println(host);

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i=0; i<5; i++){ 
    int retval = client->connect(host, httpsPort);
    if (retval == 1){
       flag = true;
       Serial.println("¡Conectado!");
       break;
    }
    else
      Serial.println("Conexión fallida. Reintentando...");
  }
  if (!flag){
    Serial.print("No pudimos conectarnos a: ");
    Serial.println(host);
    return;
  }
  delete client;    // delete HTTPSRedirect object
  client = nullptr; // delete HTTPSRedirect object

}

void loop()
{
  // Ralizar el calculo de potencia con los sensores
  //Console_Power();
  
  // Enviar_datos();
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