Öffnen Sie nun das Codebeispiel GraphicTest der Bibliothek.
Klicken Sie dazu auf:
Datei -> Beispiele -> U8g2 -> u8x8-> GraphicTest
Fügen Sie nun den folgenden Konstruktor für das Display in das Programm ein, wie im Bild unten zusehen:
U8X8_SSD1309_128X64_NONAME2_4W_SW_SPI u8x8(13, 11, 10, 9, 8);



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


#########
<!DOCTYPE html>
<html>
<style>
table, th, td {
  border:1px solid black;
  border-collapse: collapse;
}
</style>
<body>

<h2>Maischen und Läutern</h2>
<form action="/action_page.php">
<table>
  <tr>
    <th></th>
    <th>Action</th> 
    <th>Temp [&deg;C]</th>
    <th>Time [min]</th>
    <th></th>
  </tr>
  <tr>
    <td>1</td>
    <td>Einmaischen</td>
    <td><input type="number" min="40" max="99" name="fname" value="45"></td>
    <td><input type="number" min="0" max="90" name="fname" value="25"></td>
    <td></td>
  </tr>
  <tr>
    <td>2</td>
    <td>Rast</td>
    <td><input type="number" min="40" max="99" name="fname" value="62"></td>
    <td><input type="number" min="0" max="90" name="fname" value="30"></td>
    <td></td>
  </tr>
  <tr>
    <td>3</td>
    <td>Rast</td>
    <td><input type="number" min="40" max="99" name="fname" value="72"></td>
    <td><input type="number" min="0" max="90" name="fname" value="30"></td>
    <td>+ / - </td>
  </tr>
  <tr>
    <td>4</td>
    <td>Abmaischen</td>
    <td><input type="number" min="40" max="99" name="fname" value="78"></td>
    <td><input type="number" min="0" max="90" name="fname" value="0"></td>
    <td></td>
  </tr> 
</table>
<h2>Würzekochen</h2>
<table>
  <tr>
    <th></th>
    <th>Time [min]</th> 
  </tr>
  <tr>
    <th>Kochdauer gesamt:</th>
    <td><input type="number" min="0" max="180" name="fname" value="60"></td>
  </tr>
  <tr>
    <th>Hopfengabe 1:</th>
    <td><input type="number" min="0" max="180" name="fname" value="0"></td>
  </tr>
  <tr>
    <th>Hopfengabe 2:</th>
    <td><input type="number" min="0" max="180" name="fname" value="30"></td>
  </tr>
</table>
  <input type="submit" value="Submit">
</form> 

</body>
</html>

