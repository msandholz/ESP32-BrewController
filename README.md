# ESP32-BrewController


#include <Adafruit_GFX.h>
#include <Adafruit_SSD1309.h>
#include <Wire.h> // Nur erforderlich, wenn du I2C statt SPI verwenden möchtest

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 // Du kannst -1 verwenden, wenn der Reset-Pin nicht angeschlossen ist

Adafruit_SSD1309 display(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_RESET); // Für SPI-Verbindung
// Adafruit_SSD1309 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Für I2C-Verbindung

void setup() {
  Serial.begin(115200);

  // Initialisiere das Display
  if(!display.begin(SSD1309_I2C_ADDRESS, OLED_RESET)) { // Für I2C-Verbindung
  // if(!display.begin(SSD1309_I2C_ADDRESS, OLED_RESET, OLED_RESET)) { // Für SPI-Verbindung
    Serial.println(F("SSD1309 nicht gefunden"));
    while (1);
  }

  display.display(); // Lösche den Bildschirm
  delay(2000); // Kurze Verzögerung, um das Initialisieren zu beobachten

  // Zeige eine Begrüßungsnachricht auf dem Display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1309_WHITE);
  display.setCursor(0, 0);
  display.println("Hallo, ESP32!");
  display.display();
}

void loop() {
  // Hier kannst du deinen Code einfügen, um das Display zu aktualisieren
}
