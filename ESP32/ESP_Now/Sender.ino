#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

// MAC-Adresse des Empfängers
uint8_t broadcastAddress[] = {0xB0, 0xB2, 0x1C, 0xFE, 0x14, 0x18};

// Struktur für die zu sendenden Daten
typedef struct struct_message 
{
    char message[240];
    int Sequenznummer;
    float Packetverlust;
}struct_message;

// Erstelle eine struct_message mit dem Namen myData
struct_message myData;

int totalPacketsSent = 0;
int totalPacketsFailed = 0;
int connectionAttempts = 0;
float packLossRate = 0.00;

// Callback-Funktion, wenn Daten gesendet wurden
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
    totalPacketsSent++;
    if (status != ESP_NOW_SEND_SUCCESS)
    {
        totalPacketsFailed++;
    }

    // Berechnung der Paketverlustrate
    packLossRate = (totalPacketsSent > 0) ? ((float)totalPacketsFailed / (float)totalPacketsSent) * 100.0: 0.0;

    //Ausgabe des Sendestaus
    Serial.print("\r\nLetzter Paket-Sendestatus: \t");
Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Zustellung erfolgreich" : "Zustellung fehlgeschlagen");
Serial.print("Gesendete Pakete: ");
Serial.println(totalPacketsSent);
Serial.print("Fehlgeschlagene Pakete: ");
Serial.println(totalPacketsFailed);
Serial.print("Paketsverlustrate: ");
Serial.print(packLossRate);
Serial.println("%");
}

void setup() 
{
    Serial.begin(115200);

    WiFi.mode(WIFI_AP_STA);
    WiFi.channel(6);

      // Ensure WiFi is set to 802.11b (needed for LR mode)
    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
    esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_LR);

    // ESP-NOW initialisieren
    if (esp_now_init() != ESP_OK) 
    {
        Serial.println("Fehler bei der Initialisierung von ESP-NOW");
        return;
    }

    // Callback-Funktion für den Sendestatus registrieren
    esp_now_register_send_cb(OnDataSent);

    // Peer registrieren
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Fehler beim Hinzufügen des Peers");
        return;
    }
}

void loop() 
{
    // Daten für den Versand vorbereiten
    strcpy(myData.message, "Hallo Empfänger!");
    myData.Sequenznummer = totalPacketsSent;
    myData.Packetverlust = packLossRate;

    // Daten senden
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

    //Verbindungsversuche zählen
    connectionAttempts++;

    // Ausgabe des Sendestatus
    if (result == ESP_OK) 
    {
        Serial.println("Daten erfolgreich gesendet");
        Serial.print("Verbindungsversuche bis zur erfolgreichen Verbindung: ");
        Serial.println(connectionAttempts);
        //Zurücksetzen der Verbindungsversuche
        connectionAttempts = 0; 
    }
    else 
    {
        Serial.println("Fehler beim Senden der Daten");
        //Verbindungsversuche erhöhen beim fehlschlag
        connectionAttempts++; 
    }
    delay(5);
}
