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
const String hoja = "dato-crudo";
const char *server = "script.google.com";
const char *fingerprint = "";
String url = "https://" + String(server) + String("/macros/s/") + String(DEP_KEY) + "/exec?hoja=" + hoja;
HTTPClient http;
int HTTPcode;
String reponseBody;
String locationRedirect;
const bool DEBUG_HTTP = false; // No procesa los datos sino los devulve directamente en el reponse body
const int TIMEOUT = 20000;
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
 * DATA:
 * Consideraciones de espacio
 *  - El espacio disponible en el ESP32
 *  - El limite que el HTTPClient puede enviar en request body
 *  - El limite que Google Apps Script puede recibir
 *  - El tiempo en que Google Apps Script se demora en responder `http.setTimeout(15000);`
 */
const int DELAY_MUESTREO = 24; // Se debe ajsutar de manera que la cola nunca se llene
const int ARRAY_SIZE = 512; // De momento 512 genera un tamaño de datos que Google no rechaza
/**
  * MAX_MESSAGE_SIZE:
  * { "tamano": XXXX, A*3 } -> ~23
  * A="voltage_X": [B], "voltage_X": [B], -> ~33 + B*2
  * B=5*ARRAY_SIZE -> ('4096,')*ARRAY_SIZE
  * 61570 bytes con ARRAY_SIZE = 2048 -> no alcanza la memoria
  * 30850 bytes con ARRAY_SIZE = 1024 -> no alcanza la memoria
  * 15490 bytes con ARRAY_SIZE = 512  
  */
const int MAX_MESSAGE_SIZE = 25+((35+(5*ARRAY_SIZE)*2)*3); // `30774 bytes` 50 de la cadena "voltaje" y "corriente", 2 de comillas, 4 digitios mas una coma, y dos veces por ser corrtiente y voltaje
const int QUEUE_SIZE = 2; // Tamaño máximo de la cola
char messageQueue[QUEUE_SIZE][MAX_MESSAGE_SIZE]; //103256
int punteroRecolector = 0;
int frontIndex = 0; // Índice del frente de la cola
int rearIndex = 0; // Índice del final de la cola
int messageCount = 0; // Cantidad de mensajes en la cola
char json[MAX_MESSAGE_SIZE];
const bool DEBUG = true;
int lectura = 1;
int sine_array[ARRAY_SIZE]; //dummy data

// Multithreading
TaskHandle_t Task1;
TaskHandle_t Task2;

/**
 * Se conecta a WiFi y lanza los hilos
 */
void setup()
{
  
  // Inicia la comunicacion serial
  Serial.begin(115200);
  // Agregar el parametro de depuración en
  if (DEBUG_HTTP) url = url+"&debug"; 
  if (DEBUG) generarSimData(); // Comentar esta linea en producción
  Serial.println("Ventanda de tiempo:" + String(DELAY_MUESTREO*ARRAY_SIZE) + "ms");

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

  xTaskCreatePinnedToCore(
      Task1code, /* Task function. */
      "Task1",   /* name of task. */
      10000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      0,         /* priority of the task */
      &Task1,    /* Task handle to keep track of created task */
      0);        /* pin task to core 0 */
  delay(500);

  xTaskCreatePinnedToCore(
      Task2code, /* Task function. */
      "Task2",   /* name of task. */
      10000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &Task2,    /* Task handle to keep track of created task */
      1);        /* pin task to core 1 */
  delay(500);
}

void Task1code(void *pvParameters)
{
  Serial.print("HTTP corriendo en nucleo ");
  Serial.println(xPortGetCoreID());
  for (;;)
  {
    sendData();
  }
}

void Task2code(void *pvParameters)
{
  Serial.print("Recolección corriendo en nucleo ");
  Serial.println(xPortGetCoreID());
  for (;;)
  {  
    recolectar(json);
    enqueueMessage(json);
    if (punteroRecolector >= ARRAY_SIZE) punteroRecolector = 0;    
  }  
}

/**
 * El loop es remplazado por los bucles de cada tarea de hilo
*/
void loop()
{
  // Sin utilizar
}

void recolectar(char* json)
{
  Serial.println("Recoletando datos...");  
    String voltaje_Y  = "[";
  String corriente_Y  = "[";
    String voltaje_B  = "[";
  String corriente_B  = "[";  
    String voltaje_R  = "[";
  String corriente_R  = "[";      
  lectura = 1;  
  for (int i = 0; i < ARRAY_SIZE-1; i++)
  {    
    // [!] Cambiar `analogReadSim()`por `analogRead()` en producción:
      voltaje_Y += analogReadSim(PIN_Y_V);
    corriente_Y += analogReadSim(PIN_Y_A);
      voltaje_B += analogReadSim(PIN_B_V);
    corriente_B += analogReadSim(PIN_B_A);
      voltaje_R += analogReadSim(PIN_R_V);
    corriente_R += analogReadSim(PIN_R_A);            
    if (i < ARRAY_SIZE - 1 -1) // Agregar coma si no es el final
    {
        voltaje_Y += ",";
      corriente_Y += ",";
        voltaje_B += ",";
      corriente_B += ",";
        voltaje_R += ",";
      corriente_R += ",";            
    } else {
        voltaje_Y += " ";
      corriente_Y += " ";
        voltaje_B += " ";
      corriente_B += " ";
        voltaje_R += " ";
      corriente_R += " ";             
    }
    lectura++;
    delay(DELAY_MUESTREO); // Es el intervalo de tiempo entre cada muestra 
  }
  voltaje_Y   += "]";
  corriente_Y += "]";
  voltaje_B   += "]";
  corriente_B += "]";
  voltaje_R   += "]";
  corriente_R += "]";    
  Serial.print("corriente_B:");
  Serial.println(corriente_B);    
  strcpy(json, "{\"tamano\":");
  strcat(json, String(ARRAY_SIZE-1).c_str());
  strcat(json, ",\"voltaje_Y\":");
  strcat(json, voltaje_Y.c_str());
  strcat(json, ",\"corriente_Y\":");
  strcat(json, corriente_Y.c_str());
  strcat(json, ",\"voltaje_B\":");
  strcat(json, voltaje_B.c_str());
  strcat(json, ",\"corriente_B\":");
  strcat(json, corriente_B.c_str());
  strcat(json, ",\"voltaje_R\":");
  strcat(json, voltaje_R.c_str());
  strcat(json, ",\"corriente_R\":");
  strcat(json, corriente_R.c_str());
  strcat(json, "}");
  Serial.print("Datos recolectados("+String(strlen(json))+ "): ");
  Serial.println(json);
  punteroRecolector++;
}

void enqueueMessage(char* newMessage) {
  if (messageCount < QUEUE_SIZE) {
    
    char* enqueuedMessage = messageQueue[frontIndex];
    strcpy(enqueuedMessage,newMessage);
    //newMessage.toCharArray(enqueuedMessage, MAX_MESSAGE_SIZE);
    Serial.println("Datos encolados: " + String(enqueuedMessage));
    rearIndex = (rearIndex + 1) % QUEUE_SIZE; // Avanzar el índice del final de la cola
    messageCount++;
  } else {
    // La cola está llena, no se puede agregar más mensajes
    Serial.println("La cola está llena. No se puede agregar más mensajes.");
  }
}

String dequeueMessage() {
  if (messageCount > 0) {
    char* dequeuedMessage = messageQueue[frontIndex];
    String message = String(dequeuedMessage);
    frontIndex = (frontIndex + 1) % QUEUE_SIZE; // Avanzar el índice del frente de la cola
    messageCount--;    
    return message;
  } else {
    // La cola está vacía, no hay mensajes para extraer
    //Serial.println("La cola está vacía. No hay mensajes para extraer.");
    return ""; // Devuelve un mensaje con ID inválido
  }
}



void sendData()
{
  if (messageCount > 0) {
    String extractedMessage = dequeueMessage();  
    Serial.println(String("Async: extractedMessage;") + extractedMessage);      
    Enviar_GoogleAppsScript(extractedMessage);
  }
}

void Enviar_GoogleAppsScript(String payload) {
  Serial.print("Async: Enviar a Google Apps Script: ");
  Serial.println(payload);
  http.begin(url);  
  http.setTimeout(TIMEOUT); //
  http.addHeader("Content-Type", "application/json"); // Indicamos que vasmo a entregar un JSON        
  // Necesitamos agarrar los headers
  const char *headerKeys[] = {"location"};
  const size_t headerKeysCount = sizeof(headerKeys) / sizeof(headerKeys[0]);
  http.collectHeaders(headerKeys, headerKeysCount);  
  // Hacemos la peticion POST y obtenemos el codigo HTTP de respuesta
  HTTPcode = http.POST(payload);
  /**
   * HTTP client errors: https://github.com/espressif/arduino-esp32/blob/master/libraries/HTTPClient/src/HTTPClient.h
   * #define HTTPC_ERROR_CONNECTION_REFUSED  (-1)
   * #define HTTPC_ERROR_SEND_HEADER_FAILED  (-2)
   * #define HTTPC_ERROR_SEND_PAYLOAD_FAILED (-3)
   * #define HTTPC_ERROR_NOT_CONNECTED       (-4)
   * #define HTTPC_ERROR_CONNECTION_LOST     (-5)
   * #define HTTPC_ERROR_NO_STREAM           (-6)
   * #define HTTPC_ERROR_NO_HTTP_SERVER      (-7)
   * #define HTTPC_ERROR_TOO_LESS_RAM        (-8)
   * #define HTTPC_ERROR_ENCODING            (-9)
   * #define HTTPC_ERROR_STREAM_WRITE        (-10)
   * #define HTTPC_ERROR_READ_TIMEOUT        (-11)
  */
  Serial.println("Async: HTTP code:" + String(HTTPcode));

  switch (HTTPcode) {
    case 302:
      locationRedirect = http.header("location"); // Cogemos la url de redireccion
      Serial.println("Async: Redirección HTTP:" + locationRedirect);
      // Cerramios la coneción anterior y abrimos otra
      http.end();
      http.begin(locationRedirect);  
      HTTPcode = http.GET(); // Y hacemos una nueva petición peroo con GET
      Serial.println("Async: HTTP code (2):" + String(HTTPcode));
      reponseBody = http.getString();
      http.end();
      Serial.println("Async: reponseBody: "+reponseBody);  
      break;
    case -1:
      Serial.println("Async: HTTPC_ERROR_CONNECTION_REFUSED");
      http.end();
      break;
    case -11:
      Serial.println("Async: HTTPC_ERROR_READ_TIMEOUT");
      http.end();
      break;      
    default:
      Serial.println("Async: HTTPClient: No hubo redirección con POST por tanto hubo un error.");  
      http.end();
      break;
  }  

  //     
}
int inicio = 1;
int cantidad = ARRAY_SIZE;
int limite = inicio + cantidad;

int analogReadSim(int pin)
{
  return sine_array[lectura];
  //return random(4096);
}

void generarSimData(){
  for (int i = 0; i <= ARRAY_SIZE; i++) {
    const int amplitude = 4096;
    const int periods = 1;
    float value = (float)(round((amplitude/2) * sin(2 * PI * i / (periods * 256)) + 1) * 100 / 100)+(amplitude/2);
    float randomValue = random(0, 100) / 100.0;  
    float mappedValue = (randomValue * 0.2) + 0.9; 
    value=value*mappedValue;
    sine_array[i] = (int)value;
  }
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