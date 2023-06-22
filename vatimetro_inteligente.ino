/**
 * Vatimetro IoT al barrio
 * IoTalBarrio@gmail.com
 * Este código envía  
 */

#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "variables.h"

/**
 * Pines ESP32:
 * (EN) (VP) (VN)
 *  ¿?   36   39   34   35   32   33
 *       |    |    |    |    |    |
 *       YV   BV   RV   RA   BA   RA
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
 * MAX_MESSAGE_SIZE:
 * { "tamano": XXXX, A*3 } -> ~23
 * A="voltage_X": [B], "voltage_X": [B], -> ~33 + B*2
 * B=5*DIM_MUESTREO -> ('4096,')*DIM_MUESTREO
 * 61570 bytes con DIM_MUESTREO = 2048 -> no alcanza la memoria
 * 30850 bytes con DIM_MUESTREO = 1024 -> no alcanza la memoria
 * 15490 bytes con DIM_MUESTREO = 512  * 
 */
const int DIM_MUESTREO = 512; // De momento 512 genera un tamaño de datos que Google no rechaza
const int DELAY_MUESTREO = 10; // (anteriormente ARRAY_SIZE) Se debe ajsutar de manera que la cola nunca se llene
const int MAX_MESSAGE_SIZE = 25+((35+(5*DIM_MUESTREO)*2)*3); // Tamaño del mensaje JSON
const int QUEUE_SIZE = 3; // Tamaño máximo de la cola
char messageQueue[QUEUE_SIZE][MAX_MESSAGE_SIZE]; // Cola de mensajes
int noMessageQueuedHit=0; // DEBUG: para contar cuantes veces el muestreo se pierde
int punteroRecolector = 0;
int frontIndex = 0; // Índice del frente de la cola
int rearIndex = 0; // Índice del final de la cola
int messageCount = 0; // Cantidad de mensajes en la cola
char mensaje[MAX_MESSAGE_SIZE];
const bool DEBUG = true;
int lecturaSimIndex = 0;
int sine_array[DIM_MUESTREO]; //dummy data

// Web app ID del despliegue webyGoogle Apps Script del desplie
const String hoja = "dato-crudo";
const char *server = "script.google.com";
const char *fingerprint = "";
String URL = "https://" + String(server) + String("/macros/s/") + String(DEP_KEY) + "/exec?hoja=" + hoja;
HTTPClient http;
int IDpeticionHTTP = 1;
const bool DEBUG_HTTP = false; // Server-side: No procesa los datos sino los devulve directamente en el reponse body
const int TIMEOUT = 20000;

// Multithreading
TaskHandle_t TareaPeticcionHTTP;
TaskHandle_t TareaRecolectarDatos;

/**
 * Se conecta a WiFi y lanza los hilos en bucle
 */
void setup()
{  
  // Inicia la comunicacion serial
  Serial.begin(115200);
  // Agregar el parametro de depuración en  
  if (DEBUG) generarSimData(); // Comentar esta linea en producción
  Serial.println("Ventanda de tiempo:" + String(DELAY_MUESTREO*DIM_MUESTREO) + "ms");

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
      TareaPeticcionHTTPInstruccion,  /* Task function. */
      "TareaPeticcionHTTP",           /* name of task. */
      10000,                          /* Stack size of task */
      NULL,                           /* parameter of the task */
      0,                              /* priority of the task */
      &TareaPeticcionHTTP,            /* Task handle to keep track of created task */
      0);                             /* pin task to core 0 */
  delay(500);

  xTaskCreatePinnedToCore(
      TareaRecolectarDatosInstruccion,  /* Task function. */
      "TareaRecolectarDatos",           /* name of task. */
      10000,                            /* Stack size of task */
      NULL,                             /* parameter of the task */
      1,                                /* priority of the task */
      &TareaRecolectarDatos,            /* Task handle to keep track of created task */
      1);                               /* pin task to core 1 */
  delay(500);
}

void TareaPeticcionHTTPInstruccion(void *pvParameters)
{
  Serial.print("HTTP corriendo en nucleo ");
  Serial.println(xPortGetCoreID());
  for (;;)
  {
    dequeueMessage();
  }
}

void TareaRecolectarDatosInstruccion(void *pvParameters)
{
  Serial.print("Recolección corriendo en nucleo ");
  Serial.println(xPortGetCoreID());
  for (;;)
  {  
    recolectar(mensaje);
    enqueueMessage(mensaje);
    if (punteroRecolector >= DIM_MUESTREO) punteroRecolector = 0; // Al acabar el ciclo de meustreo devolver el puntero al inicio
  }  
}

/**
 * El loop es remplazado por los bucles de cada tarea de hilo
*/
void loop() { } // Sin utilizar

/**
 * Esta función recolecta DIM_MUESTREO-muestras con un retraso de DELAY_MUESTREO-milisegundos y lo guerda
 * en el arreglo al que apunta `mensajePtr`
*/
void recolectar(char* mensajePtr)
{
  Serial.println("Recoletando datos...");  
    String voltaje_Y  = "[";
  String corriente_Y  = "[";
    String voltaje_B  = "[";
  String corriente_B  = "[";  
    String voltaje_R  = "[";
  String corriente_R  = "[";      
  lecturaSimIndex = 0; // Para la función `analogReadSim()` obtener el indice del arreglo de datos simiulados
  for (int i = 0; i < DIM_MUESTREO; i++)
  { 
    /**
     * Medición de datos
     * [!] Cambiar `analogReadSim()`por `analogRead()` en producción: 
     */ 
      voltaje_Y += analogReadSim(PIN_Y_V);
    corriente_Y += analogReadSim(PIN_Y_A);
      voltaje_B += analogReadSim(PIN_B_V);
    corriente_B += analogReadSim(PIN_B_A);
      voltaje_R += analogReadSim(PIN_R_V);
    corriente_R += analogReadSim(PIN_R_A);        
    delay(DELAY_MUESTREO); // Es el intervalo de tiempo entre cada muestra 

    if (i < DIM_MUESTREO - 1) // Agregar coma si no es el final
    {
        voltaje_Y += ",";
      corriente_Y += ",";
        voltaje_B += ",";
      corriente_B += ",";
        voltaje_R += ",";
      corriente_R += ",";            
    }
    lecturaSimIndex++; // Para la función `analogReadSim()` obtener el indice del arreglo de datos simiulados
    delay(DELAY_MUESTREO); // Es el intervalo de tiempo entre cada muestra 
  }
    voltaje_Y += "]";
  corriente_Y += "]";
    voltaje_B += "]";
  corriente_B += "]";
    voltaje_R += "]";
  corriente_R += "]";    
/*   if (DEBUG) Serial.println("Datos recolectados:");
  if (DEBUG) Serial.print("voltaje_Y:"); Serial.println(voltaje_Y); Serial.print("corriente_Y:"); Serial.println(corriente_Y);
  if (DEBUG) Serial.print("voltaje_B:"); Serial.println(voltaje_B); Serial.print("corriente_B:"); Serial.println(corriente_B);
  if (DEBUG) Serial.print("voltaje_R:"); Serial.println(voltaje_R); Serial.print("corriente_R:"); Serial.println(corriente_R); */
  strcpy(mensajePtr, "{\"tamano\":");
  strcat(mensajePtr, String(DIM_MUESTREO).c_str());
  strcat(mensajePtr, ",\"voltaje_Y\":");
  strcat(mensajePtr, voltaje_Y.c_str());
  strcat(mensajePtr, ",\"corriente_Y\":");
  strcat(mensajePtr, corriente_Y.c_str());
  strcat(mensajePtr, ",\"voltaje_B\":");
  strcat(mensajePtr, voltaje_B.c_str());
  strcat(mensajePtr, ",\"corriente_B\":");
  strcat(mensajePtr, corriente_B.c_str());
  strcat(mensajePtr, ",\"voltaje_R\":");
  strcat(mensajePtr, voltaje_R.c_str());
  strcat(mensajePtr, ",\"corriente_R\":");
  strcat(mensajePtr, corriente_R.c_str());
  strcat(mensajePtr, "}");
/*   if (DEBUG) Serial.print("Mensaje Char("+String(strlen(mensaje))+ "): ");
  if (DEBUG) Serial.println(mensaje); */
  punteroRecolector++;
  encenderLED(1, 100);
}

/**
 * Esta función utiliza un apuntador a `mensaje` para guardar su valor en la cola de mensaje
*/
void enqueueMessage(char* mensajePtr) {
  if (messageCount < QUEUE_SIZE) { // La cola no está llena, se puede agregar más mensajes
    char* enqueuedMessagePtr = messageQueue[frontIndex];
    strcpy(enqueuedMessagePtr,mensajePtr);
    //Serial.println("Datos encolados: " + String(enqueuedMessagePtr));
    rearIndex = (rearIndex + 1) % QUEUE_SIZE; // Avanzar el índice del final de la cola
    messageCount++;  
    if (DEBUG) {
      size_t size = sizeof(char) * (strlen(enqueuedMessagePtr) + 1);
      Serial.print("Mensaje encolado char*("+String(size)+" bytes): ");
    }
    if (DEBUG) Serial.println(enqueuedMessagePtr);    
    if (DEBUG) {
      size_t freeHeap = heap_caps_get_free_size(MALLOC_CAP_8BIT); // Obtener el espacio en memoria libre  
      Serial.println("Espacio libre en memoria: " + String(freeHeap) + " bytes"); // Imprimir el espacio en memoria libre en bytes
    }
  } else { // La cola está llena, no se puede agregar más mensajes
    noMessageQueuedHit++;
    Serial.println("La cola está llena. No se puede agregar más mensajes. noMessageQueuedHit: " + String(noMessageQueuedHit));
  }
}


void dequeueMessage()
{
  if (messageCount > 0) {
    char* dequeuedMessage = messageQueue[frontIndex];
    frontIndex = (frontIndex + 1) % QUEUE_SIZE; // Avanzar el índice del frente de la cola
    messageCount--;    
    if (DEBUG) Serial.print("Async: extractedMessage: " + String(dequeuedMessage));         
    enviarPeticionHTTP(dequeuedMessage);    
  } 
}

void enviarPeticionHTTP(String payload) {
  String reponseBody;
  int HTTPcode;
  int HTTPcode2;
  String locationRedirect;
  Serial.print("Async: Petcción HTTP ("+String(IDpeticionHTTP)+"): ");
  Serial.println(payload);
  http.begin(URL);  
  http.setReuse(true);// Habilitar la conexión persistente: https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Keep-Alive
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
  Serial.println("Async: Petcción HTTP ("+String(IDpeticionHTTP)+"): HTTP code: " + String(HTTPcode));

  switch (HTTPcode) {
    case 302:
      locationRedirect = http.header("location"); // Cogemos la url de redireccion
      Serial.println("Async: Petcción HTTP ("+String(IDpeticionHTTP)+"): Redirección: " + locationRedirect);
      // Cerramios la coneción anterior y abrimos otra
      http.end();
      http.begin(locationRedirect);              
      HTTPcode2 = http.GET(); // Y hacemos una nueva petición peroo con GET
      Serial.println("Async: Petcción HTTP ("+String(IDpeticionHTTP)+"): Redirección: HTTP code: " + String(HTTPcode2));
      reponseBody = http.getString();
      http.end();
      Serial.println("Async: Petcción HTTP ("+String(IDpeticionHTTP)+"): Redirección: reponseBody: "+reponseBody); 
      encenderLED(2, 100); 
      break;
    case -1:
      Serial.println("Async: Petcción HTTP ("+String(IDpeticionHTTP)+"): HTTPC_ERROR_CONNECTION_REFUSED");
      http.end();
      break;
    case -11:
      Serial.println("Async: Petcción HTTP ("+String(IDpeticionHTTP)+"): HTTPC_ERROR_READ_TIMEOUT");
      http.end();
      break;      
    default:
      Serial.println("Async: Petcción HTTP ("+String(IDpeticionHTTP)+"):: No hubo redirección con POST por tanto hubo un error.");  
      http.end();
      break;    
  }    
  IDpeticionHTTP++;
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

/**
 * Simula la función `analogReadSim()`
*/
int analogReadSim(int pin)
{
  return sine_array[lecturaSimIndex];
}

/**
 * Crear una señal senosoidal
*/
int inicio = 1;
int cantidad = DIM_MUESTREO;
int limite = inicio + cantidad;
void generarSimData(){
  for (int i = 0; i <= DIM_MUESTREO; i++) {
    const int amplitude = 2700;
    const int periods = 4;
    float value = (float)(round((amplitude/2) * sin(2 * PI * i * periods / 256) + 1) * 100 / 100)+(amplitude/2);
    float randomValue = random(0, 100) / 100.0;  
    float mappedValue = (randomValue * 0.2) + 0.9; 
    value=value*mappedValue;
    sine_array[i] = (int)value;
  }
}