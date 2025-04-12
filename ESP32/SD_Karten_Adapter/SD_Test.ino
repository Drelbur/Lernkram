#include "SD.h"
#include "SPI.h"

void readFile(fs::FS &fs, const char * path) {
  Serial.printf("Lesen der Quelldatei     :%s\n", path);
  File file = fs.open(path);

  if(!file){
    Serial.println("Lesendes Öffnen der Datei gescheitert");
    return;
  }

  Serial.print("gelesener Inhalt         :");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("In Zieldatei schreiben  : %s\n", path);
  File file =fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Schreibendes Öffnen der Datei gescheitert");
    return;
  }
  if(file.print(message)){
    Serial.println("\"" +String (message) + "\" in Zieldatei geschrieben");
  } else {
    Serial.println("Schreibzugriff gescheitert");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("an Zieldatei anfügen     :%s\n", path);
  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Anfügendes Öffnen der Datei gescheitert");
    return;
  }
  if(file.print(message)){
    Serial.println("Text an Dateiinhalt angefügt");
  } else {
    Serial.println("Anfügen gescheitert");
  }
  file.close();
}

void setup(){
  Serial.begin(115200);
  delay (50);
  if (!SD.begin()){
    Serial.println("Mount der SD-Karte gescheitert");
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE){
    Serial.println("Kartentyp nicht erkannt");
    return;
  }

  writeFile(SD, "/test.txt", "Hallo ");
  appendFile(SD, "/test.txt", "ESP32 SD-Karte\n");
  readFile(SD, "/test.txt");
}

void loop(){

}
