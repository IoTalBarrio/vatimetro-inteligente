//----------------------------
#include <WiFi.h>
#include <HTTPClient.h>
#include "EmonLib.h"   //https://github.com/openenergymonitor/EmonLib
//----------------------------


//Credenciales
const char * ssid = "FLOR";
const char * password = "39769017";
String GOOGLE_SCRIPT_ID = "AKfycbxYAnZeaAvqqrL8fpogsdGQjKbhXuKFGsKxmN2v4cL1jrwigu7KGvVtYnmsEQv3Iiec1Q";
//---------------------------------------------------------------------



//Calibracion de corriente y tension
EnergyMonitor emon;
#define vCalibration 164.1
#define currCalibration 2.38974
#define PowerFactor 1.7  //????? -> Falta calibrar

 void setup() {

  //Inicia la comunicacion serial
  Serial.begin(115200);

  //Configuracion de los sensores
  emon.voltage(36, vCalibration, PowerFactor); // Voltage: input pin, calibration, phase_shift
  emon.current(39, currCalibration); // Current: input pin, calibration.

  //Calculos de prueba
  emon.calcVI(500, 5000);
  emon.calcVI(500, 5000);
  emon.calcVI(500, 5000);

  
  delay(10);

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
    emon.calcVI(500, 5000);
    Serial.print("Vrms: ");
    Serial.print(emon.Vrms, 2);
    Serial.print("V");
    Serial.print("\tIrms: ");
    Serial.print(emon.Irms, 4);
    Serial.print("A");
    Serial.print("\tPotencia Aparente: ");
    Serial.print(emon.apparentPower, 4);
    Serial.print("VA");
    Serial.print("\tPotencia Real: ");
    Serial.print(emon.realPower, 4);
    Serial.print("W");
    Serial.print("\tFactor de potencia: ");
    Serial.print(emon.powerFactor, 4);
    Serial.println("W");
}


void Send_Data()
{

    String param;
    param  = "vrms="+String(emon.Vrms,5);                 //  Tension RMS
    param += "&irms="+String(emon.Irms,8);                //  Corriente RMS
    param += "&fp="+String(emon.powerFactor,5);           //  Factor de potencia
    param += "&preal="+String(emon.realPower,5);          //  Potencia Real
    param += "&paparente="+String(emon.apparentPower,5);  //  Potencia aparente

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