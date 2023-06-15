//Link de la hoja de calculo
//https://docs.google.com/spreadsheets/d/1_Xl2GG_nV-WEvq9iGkfc7-1aAKoTG6-fI5tBE2JrKrY/edit#gid=0

var ss = SpreadsheetApp.openById('1_Xl2GG_nV-WEvq9iGkfc7-1aAKoTG6-fI5tBE2JrKrY'); //ID de la hoja de calculo
var sheet = ss.getSheetByName('Hoja 1');                                          //Hoja donde se guardan los datos

//################################ Funcion que obtiene los datos #################################
//write_google_sheet() function in esp32 sketch, is send data to this code block

function doGet(e){
  //get data from ESP32
  if (e.parameter == 'undefined') {
    return ContentService.createTextOutput("Received data is undefined");
  }

  var dateTime = new Date();
  vrms              = e.parameters.vrms;
  irms              = e.parameters.irms;
  potencia_factor   = e.parameters.fp;
  potencia_real     = e.parameters.preal;
  potencia_aparente = e.parameters.paparente;

  //----------------------------------------------------------------------------------
  var nextRow = sheet.getLastRow() + 1;
  sheet.getRange("A" + nextRow).setValue(dateTime);
  sheet.getRange("B" + nextRow).setValue(dateTime);
  sheet.getRange("C" + nextRow).setValue(vrms);
  sheet.getRange("D" + nextRow).setValue(irms);
  sheet.getRange("E" + nextRow).setValue(potencia_factor);
  sheet.getRange("F" + nextRow).setValue(potencia_real);
  sheet.getRange("G" + nextRow).setValue(potencia_aparente);
  
  //----------------------------------------------------------------------------------

  //returns response back to ESP32
  return ContentService.createTextOutput("Status Updated in Google Sheet");
  //----------------------------------------------------------------------------------
}

//AKfycbytctQT6ppIJInuSKd4at8Pm3ClXlhKHkz_4YFrdZSzYtnUWN2RbCOe7rS0h1oEVp93Iw ID
