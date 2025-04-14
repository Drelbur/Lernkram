#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <SD.h>
#include <SPI.h>

typedef struct struct_message 
{
    char message[240];
    int Sequenznummer;
    float packetverlust;
}struct_message;

struct_message myData;

//variablen für die Datenrate
unsigned long startTime = millis();
unsigned long packetCount = 0;

// Globale Variablen für gespeicherte Daten
char lastMacStr[18] = "";
int8_t lastRssi = 0;
int lastLen = 0;
char lastMessage[240] = "";
int lastSequenznumber = 0;
float lastPackLossRate = 0.00;
bool newDataReceived = false;

void OnDataRecv(const esp_now_recv_info_t* msg_info, const uint8_t* incomingData, int len) {
    memcpy(&myData, incomingData, sizeof(myData));
    packetCount++;

    // Daten in globale Variablen speichern
    snprintf(lastMacStr, sizeof(lastMacStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             msg_info->src_addr[0], msg_info->src_addr[1],
             msg_info->src_addr[2], msg_info->src_addr[3],
             msg_info->src_addr[4], msg_info->src_addr[5]);
    
    lastRssi = msg_info->rx_ctrl->rssi;
    lastLen = len;
    memcpy(lastMessage, myData.message, sizeof(lastMessage));
    lastSequenznumber = myData.Sequenznummer;
    lastPackLossRate = myData.packetverlust;
    newDataReceived = true;
}

void printDataRate() {
    unsigned long currentTime = millis();
    float elapsedSeconds = (currentTime - startTime) / 1000.0;
    float dataRate = (packetCount * sizeof(struct_message)) / elapsedSeconds;

    // Immer Datenrate ausgeben
    Serial.printf("Datenrate: %.2f Byte/s | %.2f kbit/s\n", 
                  dataRate, dataRate * 8 / 1024);

    // Nur bei neuen Daten Details anzeigen
    if (newDataReceived) {
        Serial.print("Letzte MAC: ");
        Serial.println(lastMacStr);
        appendFile(SD, "/Empfänger_Daten.txt", "Letzte Mac: " + lastMacStr + "\n");
        Serial.print("RSSI: ");
        Serial.print(lastRssi);
        Serial.println(" dBm");
        appendFile(SD, "/Empfänger_Daten.txt", "RSSI: " + lastRssi + "\n");
        Serial.print("Bytes: ");
        Serial.println(lastLen);
        appendFile(SD, "/Empfänger_Daten.txt", "Bytes: " + lastLen + "\n");
        Serial.print("Nachricht: ");
        Serial.println(lastMessage);
        appendFile(SD, "/Empfänger_Daten.txt", "Nachricht: " + lastMessage + "\n");
        Serial.print("Sequenz: ");
        Serial.println(lastSequenznumber);
        appendFile(SD, "/Empfänger_Daten.txt", "Sequenz: " + lastSequenznumber + "\n");
        Serial.print("Packetverlust: ");
        Serial.print(lastPackLossRate);
        Serial.println("%");
        appendFile(SD, "/Empfänger_Daten.txt", "Pacetverlust: " + lastPackLossRate + "\n");
        Serial.println("-------------------");
        
        newDataReceived = false;  // Zurücksetzen
    }

    packetCount = 0;
    startTime = currentTime;
}

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

void setup() 
{
    Serial.begin(115200);
    WiFi.mode(WIFI_AP_STA);
    WiFi.channel(6);

      // Ensure WiFi is set to 802.11b (needed for LR mode)
    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
    esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_LR);

    if (esp_now_init() != ESP_OK) 
    {
        Serial.println("ESP-NOW Initialisierung fehlgeschlagen");
        return;
    }

    esp_now_register_recv_cb(OnDataRecv);
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
  writeFile(SD, "/Empfänger_Daten.txt", "Beginn:\n");
  appendFile(SD, "/test.txt", "ESP32 SD-Karte\n");
  readFile(SD, "/test.txt");
}

void loop() 
{
    static unsigned long lastPrint = 0;

    if (millis() - lastPrint >= 10000) 
    { // Alle 10 Sekunden ausgeben
        printDataRate();
        lastPrint = millis();
    }
}
