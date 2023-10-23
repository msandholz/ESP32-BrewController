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
<head>
<title>Page Title</title>
<script>
// transform Brix to Plato
function Brix2Plato()
{
	var brx = document.getElementById("brix").value;
	document.getElementById("plato").innerHTML = brx * 0.962;
}

</script>
  <style>
    .box {
    	border-width: 1px;
    	border-style: solid;
    }

  </style>

</head>
<body>

<div class="box">
        <h3>Umrechnung von Brix nach Plato</h3>
        <form name="f_density">
		
			<table style="border-style:none;">
				<tbody><tr>
					<td>Gemessene Stammwürze (°Brix)</td>
					<td><input class="infield" type="text" name="density_input" value="13" id="brix" onkeyup="javascript:Brix2Plato()"></td>
				</tr>
				<tr>
				  <td>Gemessene Stammwürze (°Plato):</td>
				  <td><div class="result" id="plato">12.506</div></td>
				</tr>
                <tr>
            		<td>Gemessene Temperatur (°C):</td>
            		<td><input class="infield" type="text" name="temp" value="20" onkeyup="javascript:_correct_wort()"></td>
            	</tr>
                <tr>
            		<td>Tatsächliche Stammwürze (°Plato):</td>
            		<td><div class="infield" type="text" name="wort" value="12" onkeyup="javascript:_correct_wort()">223</div></td>
            	</tr>
			</tbody></table>
        </form>
</div>



</body>
</html>


