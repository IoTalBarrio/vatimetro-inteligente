//----------------------------
#include <WiFi.h>
#include <HTTPClient.h>
#include "EmonLib.h"   //https://github.com/openenergymonitor/EmonLib
#include "variables.h"
//----------------------------


//Credenciales
const char * ssid = SSID;
const char * password = PASSWORD;
String GOOGLE_SCRIPT_ID = DEP_KEY;
//---------------------------------------------------------------------


//Calibracion de corriente y tension
EnergyMonitor emon_1;
EnergyMonitor emon_2;
EnergyMonitor emon_3;

#define vCalibration_1 39.44
#define currCalibration_1 8.1145913
#define PowerFactor_1 1.7  //????? -> Falta calibrar


#define vCalibration_2 27.4
#define currCalibration_2 5.91999
#define PowerFactor_2 1.7  //????? -> Falta calibrar


#define vCalibration_3 13
#define currCalibration_3 0.28786
#define PowerFactor_3 1.7  //????? -> Falta calibrar




 void setup() {

  //Inicia la comunicacion serial
  Serial.begin(115200);

  //Configuracion de los sensores
  emon_1.voltage(36, vCalibration_1, PowerFactor_1); // Voltage: input pin, calibration, phase_shift
  emon_1.current(35, currCalibration_1); // Current: input pin, calibration.

  emon_2.voltage(39, vCalibration_2, PowerFactor_2); // Voltage: input pin, calibration, phase_shift
  emon_2.current(32, currCalibration_2); // Current: input pin, calibration.

  emon_3.voltage(34, vCalibration_3, PowerFactor_3); // Voltage: input pin, calibration, phase_shift
  emon_3.current(33, currCalibration_3); // Current: input pin, calibration.

  //Calculos de prueba
  emon_1.calcVI(500, 5000);
  emon_1.calcVI(500, 5000);
  emon_1.calcVI(500, 5000);

  emon_2.calcVI(500, 5000);
  emon_2.calcVI(500, 5000);
  emon_2.calcVI(500, 5000);

  emon_3.calcVI(500, 5000);
  emon_3.calcVI(500, 5000);
  emon_3.calcVI(500, 5000);

  //Conexion con la red WIFI
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("OK");
}




void loop() {
  //Ralizar el calculo de potencia con los sensores
  Console_Power();
  //Enviar los datos a google sheets
  Send_Data();
}


void Console_Power() {
    emon_1.calcVI(500, 5000);
    emon_2.calcVI(500, 5000);
    emon_3.calcVI(500, 5000);

    Serial.println("Primer Par de sensores:");
    Serial.print("Vrms: ");
    Serial.print(emon_1.Vrms, 2);
    Serial.print("V");
    Serial.print("\tIrms: ");
    Serial.print(emon_1.Irms, 4);
    Serial.print("A");
    Serial.print("\tPotencia Aparente: ");
    Serial.print(emon_1.apparentPower, 4);
    Serial.print("VA");
    Serial.print("\tPotencia Real: ");
    Serial.print(emon_1.realPower, 4);
    Serial.print("W");
    Serial.print("\tFactor de potencia: ");
    Serial.print(emon_1.powerFactor, 4);
    Serial.println("W");

    Serial.println("Segundo Par de sensores:");
    Serial.print("Vrms: ");
    Serial.print(emon_2.Vrms, 2);
    Serial.print("V");
    Serial.print("\tIrms: ");
    Serial.print(emon_2.Irms, 4);
    Serial.print("A");
    Serial.print("\tPotencia Aparente: ");
    Serial.print(emon_2.apparentPower, 4);
    Serial.print("VA");
    Serial.print("\tPotencia Real: ");
    Serial.print(emon_2.realPower, 4);
    Serial.print("W");
    Serial.print("\tFactor de potencia: ");
    Serial.print(emon_2.powerFactor, 4);
    Serial.println("W");

    Serial.println("Tercer Par de sensores:");
    Serial.print("Vrms: ");
    Serial.print(emon_3.Vrms, 2);
    Serial.print("V");
    Serial.print("\tIrms: ");
    Serial.print(emon_3.Irms, 4);
    Serial.print("A");
    Serial.print("\tPotencia Aparente: ");
    Serial.print(emon_3.apparentPower, 4);
    Serial.print("VA");
    Serial.print("\tPotencia Real: ");
    Serial.print(emon_3.realPower, 4);
    Serial.print("W");
    Serial.print("\tFactor de potencia: ");
    Serial.print(emon_3.powerFactor, 4);
    Serial.println("W");
}


void Send_Data()
{
    //Primer par
    String param;
    param  = "vrmsY="+String(emon_1.Vrms,5);                 //  Tension RMS
    param += "&irmsY="+String(emon_1.Irms,8);                //  Corriente RMS
    param += "&fpY="+String(emon_1.powerFactor,5);           //  Factor de potencia
    param += "&prealY="+String(emon_1.realPower,5);          //  Potencia Real
    param += "&paparenteY="+String(emon_1.apparentPower,5);  //  Potencia aparente
    
    //Segundo par
    param += "&vrmsB="+String(emon_2.Vrms,5);                 //  Tension RMS
    param += "&irmsB="+String(emon_2.Irms,8);                //  Corriente RMS
    param += "&fpB="+String(emon_2.powerFactor,5);           //  Factor de potencia
    param += "&prealB="+String(emon_2.realPower,5);          //  Potencia Real
    param += "&paparenteB="+String(emon_2.apparentPower,5);  //  Potencia aparente
    
    //Tercer par
    param += "&vrmsR="+String(emon_3.Vrms,5);                 //  Tension RMS
    param += "&irmsR="+String(emon_3.Irms,8);                //  Corriente RMS
    param += "&fpR="+String(emon_3.powerFactor,5);           //  Factor de potencia
    param += "&prealR="+String(emon_3.realPower,5);          //  Potencia Real
    param += "&paparenteR="+String(emon_3 .apparentPower,5);  //  Potencia aparente

    param.replace(".", ",");

    Serial.println(param);
    write_to_google_sheet(param);

}

void write_to_google_sheet(String params) {
   HTTPClient http;
   String url="https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+params;
    Serial.println("Subiendo datos a google sheets");

    //starts posting data to google sheet
    http.begin(url.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();  
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);
    
    //getting response from google sheet
  
    String payload;
    if (httpCode > 0) {
        payload = http.getString();
        Serial.println("Payload: "+payload);     
    }

    http.end();
}