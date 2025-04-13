/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-micro-sd-card
 */

#include <SD.h>

#define PIN_SPI_CS 4

File myFile;

void setup() {
  Serial.begin(9600);

  if (!SD.begin(PIN_SPI_CS)) {
    Serial.println(F("SD CARD FAILED, OR NOT PRESENT!"));
    while (1); // don't do anything more:
  }

  Serial.println(F("SD CARD INITIALIZED."));

  if (!SD.exists("arduino.txt")) {
    Serial.println(F("arduino.txt doesn't exist. Creating arduino.txt file..."));
    // create a new file by opening a new file and immediately close it
    myFile = SD.open("arduino.txt", FILE_WRITE);
    myFile.close();
  }

  // recheck if file is created or not
  if (SD.exists("arduino.txt"))
    Serial.println(F("arduino.txt exists on SD Card."));
  else
    Serial.println(F("arduino.txt doesn't exist on SD Card."));
}

void loop() {
}
