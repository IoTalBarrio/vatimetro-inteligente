/**
 * Vatimetro IoT al barrio
 * IoTalBarrio@gmail.com
 */

//----------------------------
#include <WiFi.h>
#include <HTTPClient.h>
#include "EmonLib.h" //https://github.com/openenergymonitor/EmonLib
#include "variables.h"
//----------------------------

// Conexión WIFI
const char *ssid = SSID;
const char *password = PASSWORD;

// Web app ID del despliegue webyGoogle Apps Script del desplie
String GOOGLE_SCRIPT_ID = DEP_KEY;



EnergyMonitor emon_Y;
EnergyMonitor emon_B;
EnergyMonitor emon_R;

// Calibracion de corriente y tension
#define vCalibration_Y 39.44
#define currCalibration_Y 8.1145913
#define PowerFactor_Y 1.7 //????? -> Falta calibrar

#define vCalibration_B 27.4
#define currCalibration_B 5.91999
#define PowerFactor_B 1.7 //????? -> Falta calibrar

#define vCalibration_R 12.8272474981946
#define currCalibration_R 0.28786
#define PowerFactor_R 1.7 //????? -> Falta calibrar

// Pines
/*
(EN) (VP) (VN)
 ¿?   36   39   34   35   32   33
      |    |    |    |    |    |
      YV   YA   RV   RA   BA   RA
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

  // Configuracion de los sensores
  emon_Y.voltage(PIN_Y_V, vCalibration_Y, PowerFactor_Y); // Voltage: input pin, calibration, phase_shift
  emon_Y.current(PIN_Y_A, currCalibration_Y);             // Current: input pin, calibration.

  emon_B.voltage(PIN_B_V, vCalibration_B, PowerFactor_B); // Voltage: input pin, calibration, phase_shift
  emon_B.current(PIN_B_A, currCalibration_B);             // Current: input pin, calibration.

  emon_R.voltage(PIN_R_V, vCalibration_R, PowerFactor_R); // Voltage: input pin, calibration, phase_shift
  emon_R.current(PIN_R_A, currCalibration_R);             // Current: input pin, calibration.

  // Calculos de prueba
  emon_Y.calcVI(500, 5000);
  emon_Y.calcVI(500, 5000);
  emon_Y.calcVI(500, 5000);

  emon_B.calcVI(500, 5000);
  emon_B.calcVI(500, 5000);
  emon_B.calcVI(500, 5000);

  emon_R.calcVI(500, 5000);
  emon_R.calcVI(500, 5000);
  emon_R.calcVI(500, 5000);

  // Conexion con la red WIFI
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Led de depuración
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    encenderLED(1, 1000);
  }
  Serial.println("OK");
  encenderLED(5, 100);
}

void loop()
{
  // Ralizar el calculo de potencia con los sensores
  Console_Power();
  // Enviar los datos a google sheets
  Send_Data();
}

void Console_Power()
{
  emon_Y.calcVI(500, 5000);
  emon_B.calcVI(500, 5000);
  emon_R.calcVI(500, 5000);

  Serial.println("Primer Par de sensores:");
  Serial.print("Vrms: ");
  Serial.print(emon_Y.Vrms, 2);
  Serial.print("V");
  Serial.print("\tIrms: ");
  Serial.print(emon_Y.Irms, 4);
  Serial.print("A");
  Serial.print("\tPotencia Aparente: ");
  Serial.print(emon_Y.apparentPower, 4);
  Serial.print("VA");
  Serial.print("\tPotencia Real: ");
  Serial.print(emon_Y.realPower, 4);
  Serial.print("W");
  Serial.print("\tFactor de potencia: ");
  Serial.print(emon_Y.powerFactor, 4);
  Serial.println("W");

  Serial.println("Segundo Par de sensores:");
  Serial.print("Vrms: ");
  Serial.print(emon_B.Vrms, 2);
  Serial.print("V");
  Serial.print("\tIrms: ");
  Serial.print(emon_B.Irms, 4);
  Serial.print("A");
  Serial.print("\tPotencia Aparente: ");
  Serial.print(emon_B.apparentPower, 4);
  Serial.print("VA");
  Serial.print("\tPotencia Real: ");
  Serial.print(emon_B.realPower, 4);
  Serial.print("W");
  Serial.print("\tFactor de potencia: ");
  Serial.print(emon_B.powerFactor, 4);
  Serial.println("W");

  Serial.println("Tercer Par de sensores:");
  Serial.print("Vrms: ");
  Serial.print(emon_R.Vrms, 2);
  Serial.print("V");
  Serial.print("\tIrms: ");
  Serial.print(emon_R.Irms, 4);
  Serial.print("A");
  Serial.print("\tPotencia Aparente: ");
  Serial.print(emon_R.apparentPower, 4);
  Serial.print("VA");
  Serial.print("\tPotencia Real: ");
  Serial.print(emon_R.realPower, 4);
  Serial.print("W");
  Serial.print("\tFactor de potencia: ");
  Serial.print(emon_R.powerFactor, 4);
  Serial.println("W");
}

void Send_Data()
{
  // Primer par
  String param;
  param = "vrmsY=" + String(emon_Y.Vrms, 5);                 //  Tension RMS
  param += "&irmsY=" + String(emon_Y.Irms, 8);               //  Corriente RMS
  param += "&fpY=" + String(emon_Y.powerFactor, 5);          //  Factor de potencia
  param += "&prealY=" + String(emon_Y.realPower, 5);         //  Potencia Real
  param += "&paparenteY=" + String(emon_Y.apparentPower, 5); //  Potencia aparente

  // Segundo par
  param += "&vrmsB=" + String(emon_B.Vrms, 5);               //  Tension RMS
  param += "&irmsB=" + String(emon_B.Irms, 8);               //  Corriente RMS
  param += "&fpB=" + String(emon_B.powerFactor, 5);          //  Factor de potencia
  param += "&prealB=" + String(emon_B.realPower, 5);         //  Potencia Real
  param += "&paparenteB=" + String(emon_B.apparentPower, 5); //  Potencia aparente

  // Tercer par
  param += "&vrmsR=" + String(emon_R.Vrms, 5);               //  Tension RMS
  param += "&irmsR=" + String(emon_R.Irms, 8);               //  Corriente RMS
  param += "&fpR=" + String(emon_R.powerFactor, 5);          //  Factor de potencia
  param += "&prealR=" + String(emon_R.realPower, 5);         //  Potencia Real
  param += "&paparenteR=" + String(emon_R.apparentPower, 5); //  Potencia aparente

  param.replace(".", ",");

  Serial.println(param);
  write_to_google_sheet(param);
}

void write_to_google_sheet(String params)
{
  HTTPClient http;
  String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?" + params;
  Serial.println("Subiendo datos a google sheets");

  // starts posting data to google sheet
  http.begin(url.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);

  // Respuesta de Google Apps Scripts Web
  String payload;
  if (httpCode > 0)
  {
    payload = http.getString();
    Serial.println("Cuerpo de respuesta HTTP: " + payload);
    encenderLED(3, 500);
  }
  else
  {
    Serial.println("No hay respuesta");
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