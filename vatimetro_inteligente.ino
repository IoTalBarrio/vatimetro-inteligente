/**
 * Vatimetro IoT al barrio
 * IoTalBarrio@gmail.com
 */

//----------------------------
#include <ArduinoJson.h>
#include "variables.h"

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
const int DELAY_MUESTREO = 3;
const int ARRAY_SIZE = 4096;
const int MAX_MESSAGE_SIZE = 50+(2+ARRAY_SIZE*5)*2; // 50 de la cadena "voltaje" y "corriente", 2 de comillas, 4 digitios mas una coma, y dos veces por ser corrtiente y voltaje
const int QUEUE_SIZE = 2; // Tamaño máximo de la cola
char messageQueue[QUEUE_SIZE][MAX_MESSAGE_SIZE]; //103256
int punteroRecolector = 0;
int frontIndex = 0; // Índice del frente de la cola
int rearIndex = 0; // Índice del final de la cola
int messageCount = 0; // Cantidad de mensajes en la cola

// Multithreading
TaskHandle_t Task1;
TaskHandle_t Task2;

/**
 *
 */
void setup()
{

  // Inicia la comunicacion serial
  Serial.begin(115200);
  Serial.flush();

  Serial.println("Ventanda de tiempo:" + String(DELAY_MUESTREO*ARRAY_SIZE) + "ms");

  // Led de depuración
  pinMode(LED_BUILTIN, OUTPUT);

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
    enqueueMessage(recolectar());
    if (punteroRecolector >= ARRAY_SIZE) punteroRecolector = 0;    
  }  
}

void loop()
{
}

String recolectar()
{
  Serial.println("Recoletando datos...");
  String json = "";
  String voltaje_string = "[";
  String corriente_string = "[";  
  for (int i = 0; i < ARRAY_SIZE; i++)
  {    
    voltaje_string +=  analogReadSim(PIN_Y_V);
    corriente_string += analogReadSim(PIN_Y_A);
    delay(DELAY_MUESTREO);  
    if (i < ARRAY_SIZE - 1) // Agregar coma si no es el final
    {
      voltaje_string += ",";
      corriente_string += ",";
    }
  }
  voltaje_string += "]";
  corriente_string += "]";
  punteroRecolector++;
  json = String("{") + String("'voltaje:'") + voltaje_string + String(", ") + String("'corriente':") + corriente_string + String("}");
  return json;
}

void enqueueMessage(String newMessage) {
  if (messageCount < QUEUE_SIZE) {
    char* enqueuedMessage = messageQueue[frontIndex];
    newMessage.toCharArray(enqueuedMessage, MAX_MESSAGE_SIZE);
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
    Serial.println(String("Mensaje;") + message);  
    delay(5000);  // simular respuesta
    return message;
  } else {
    // La cola está vacía, no hay mensajes para extraer
    //Serial.println("La cola está vacía. No hay mensajes para extraer.");
    // Puedes devolver un mensaje de error o un valor nulo, según tus necesidades
    return ""; // Devuelve un mensaje con ID inválido
  }
}


void sendData()
{
  String extractedMessage = dequeueMessage();  
}

int analogReadSim(int pin)
{
  return random(4096);
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