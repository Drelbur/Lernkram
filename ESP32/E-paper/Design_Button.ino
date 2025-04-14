#include <GxEPD2_3C.h>
#include <U8g2_for_Adafruit_GFX.h>

#define CS_PIN 15
#define DC_PIN 27
#define RST_PIN 26
#define BUSY_PIN 25
const int Button_pin = 23;

GxEPD2_3C<GxEPD2_750c_Z08, GxEPD2_750c_Z08::HEIGHT> display(GxEPD2_750c_Z08(CS_PIN, DC_PIN, RST_PIN, BUSY_PIN));
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

// Variablen Deklaration
int buttonState = 0;
int statusIndex = 0;
const char* raumStatus[3] = {"Abwesend", "Anwesend", "Bitte nicht Stören!"};

String Raumstatus ="Abwesend";
String RaumID = "B 128";
String Bereich = "Geschäftsleitung";
String Vorname = "Andreas";
String Nachname = "Kather";
String Stellung = "Geschäftsführer";
String Telefonnummer = "+49 30 36502-122";
String Mobilnummer = "017683387049";

void setup() {
  Serial.begin(115200);
  pinMode(Button_pin, INPUT);
  SPI.begin(13, -1, 14);
  display.init();
  display.setRotation(0);
  u8g2Fonts.begin(display);
  updateDisplay();
  display.hibernate();
}

void updateDisplay() {
  display.setFullWindow();
  display.firstPage();
  do {
    drawContent();
  } while (display.nextPage());
  display.hibernate();
}

void drawContent() {
  display.fillScreen(GxEPD_WHITE);
  display.fillRect(0, 0, 800 * 0.2, 480, GxEPD_RED);
  display.fillRect(200, 80, 500, 5, GxEPD_RED);
  
  // Erste Schriftart
  u8g2Fonts.setFont(u8g2_font_fub35_tf);
  u8g2Fonts.setFontMode(1);
  u8g2Fonts.setForegroundColor(GxEPD_WHITE);
  u8g2Fonts.setCursor(10, 50);
  u8g2Fonts.print(RaumID);
  
  u8g2Fonts.setForegroundColor(GxEPD_BLACK);
  u8g2Fonts.setCursor(250, 50);
  u8g2Fonts.print(Bereich);
  
  // Zweite Schriftart
  u8g2Fonts.setFont(u8g2_font_fub20_tf);
  u8g2Fonts.setFontMode(1);
  u8g2Fonts.setCursor(250, 150);
  u8g2Fonts.print(Vorname + " " + Nachname);
  
  u8g2Fonts.setCursor(250, 200);
  u8g2Fonts.print(Stellung);
  
  u8g2Fonts.setCursor(250, 250);
  u8g2Fonts.print("Tel: " + Telefonnummer);
  
  u8g2Fonts.setCursor(250, 300);
  u8g2Fonts.print("Mobil: " + Mobilnummer);

  u8g2Fonts.setCursor(600, 450);
  u8g2Fonts.print(Raumstatus);
}

// Angepasster Code-Ausschnitt
int updateCounter = 0;

void loop() 
{
  buttonState = digitalRead(Button_pin);
  Serial.println(buttonState);
  if (buttonState == HIGH) 
  {
    statusIndex = (statusIndex + 1) % 3;
    Raumstatus = raumStatus[statusIndex];
    
      display.init();
      display.setFullWindow();
      display.firstPage();
      do {
        drawContent();
      } while (display.nextPage());
    
    display.hibernate();
  }
}
