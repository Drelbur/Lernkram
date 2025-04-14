#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

// Struktur für die empfangenen Daten
typedef struct struct_message 
{
    char message[230];
    int Sequenznummer;
    float Packetverlust;
    int currentChannel;
    bool ackReceived;
} struct_message;

struct_message myData;

// Variablen für die Datenrate
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

// Kanalvariablen
int currentChannel = 6;
const int maxChannels = 13;
unsigned long lastDataReceived = millis();
unsigned long lastChannelSwitch = millis();

// Callback-Funktion für empfangene Daten
void OnDataRecv(const esp_now_recv_info_t* msg_info, const uint8_t* incomingData, int len) 
{
    memcpy(&myData, incomingData, sizeof(myData));
    packetCount++;

    // Synchronisierter Kanalwechsel, wenn Nachricht "Kanalwechsel" empfangen wird
    if (strcmp(myData.message, "Kanalwechsel") == 0) 
    {
        if (myData.currentChannel != currentChannel) 
        {
            currentChannel = myData.currentChannel;
            WiFi.setChannel(currentChannel);
            Serial.printf("Synchronisierter Wechsel zu Kanal %d basierend auf Sender.\n", currentChannel);
        }
    }

    // Daten in globale Variablen speichern
    snprintf(lastMacStr, sizeof(lastMacStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             msg_info->src_addr[0], msg_info->src_addr[1],
             msg_info->src_addr[2], msg_info->src_addr[3],
             msg_info->src_addr[4], msg_info->src_addr[5]);
    
    lastRssi = msg_info->rx_ctrl->rssi;
    lastLen = len;
    memcpy(lastMessage, myData.message, sizeof(lastMessage));
    lastSequenznumber = myData.Sequenznummer;
    lastPackLossRate = myData.Packetverlust;
    newDataReceived = true;

    // Aktualisierung des Zeitstempels für den letzten Empfang
    lastDataReceived = millis();
}

// Funktion zur Ausgabe der Datenrate
void printDataRate() 
{
    unsigned long currentTime = millis();
    float elapsedSeconds = (currentTime - startTime) / 1000.0;
    float dataRate = (packetCount * sizeof(struct_message)) / elapsedSeconds;

    // Immer Datenrate ausgeben
    Serial.printf("Datenrate: %.2f Byte/s | %.2f kbit/s\n", 
                  dataRate, dataRate * 8 / 1024);

    // Nur bei neuen Daten Details anzeigen
    if (newDataReceived) 
    {
        Serial.print("Letzte MAC: ");
        Serial.println(lastMacStr);
        Serial.print("RSSI: ");
        Serial.print(lastRssi);
        Serial.println(" dBm");
        Serial.print("Bytes: ");
        Serial.println(lastLen);
        Serial.print("Nachricht: ");
        Serial.println(lastMessage);
        Serial.print("Sequenz: ");
        Serial.println(lastSequenznumber);
        Serial.print("Packetverlust: ");
        Serial.print(lastPackLossRate);
        Serial.println("%");
        Serial.println("-------------------");
        
        newDataReceived = false;  
    }

    packetCount = 0;
    startTime = currentTime;
}

void setup() 
{
    Serial.begin(115200);
    WiFi.mode(WIFI_AP_STA);
    WiFi.channel(currentChannel);

    // Ensure WiFi is set to 802.11b (needed for LR mode)
    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
    esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_LR);

    if (esp_now_init() != ESP_OK) 
    {
        Serial.println("ESP-NOW Initialisierung fehlgeschlagen");
        return;
    }

    esp_now_register_recv_cb(OnDataRecv);
}

void loop() 
{
    static unsigned long lastPrint = 0;

    // Datenrate alle 10 Sekunden ausgeben
    if (millis() - lastPrint >= 10000) 
    {
        printDataRate();
        lastPrint = millis();
    }

    // Unabhängiger Kanalwechsel nach 15 Sekunden ohne Empfang
    if (millis() - lastDataReceived > 15000) 
    {
        if (millis() - lastChannelSwitch > 15000) 
        {
            currentChannel++;
            if (currentChannel > maxChannels) 
            {
                currentChannel = 1; 
            }
            WiFi.setChannel(currentChannel);
            Serial.printf("Kein Empfang. Unabhängiger Wechsel zu Kanal %d\n", currentChannel);
            lastChannelSwitch = millis();
        }
    }
}
