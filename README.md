https://microcontrollerslab.com/esp32-dual-core-freertos-arduino-ide/


NTP:
https://randomnerdtutorials.com/esp32-date-time-ntp-client-server-arduino/
https://lastminuteengineers.com/esp32-ntp-server-date-time-tutorial/
https://github.com/SensorsIot/NTP-time-for-ESP8266-and-ESP32

Europe	Berlin,Germany	CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00

time_t now;
tm timeinfo;



time(&now);
localtime_r(&now, &imeinfo);
long unsigned lastNTPtime;
unsigned long lastEntryTime;

void setup() {
  configTime(0,0,NTP-SERVER);
  setenv("TZ", TZ_INFO, 1);
 getTimeReducedTraffic(3600);
  showTime(timeinfo);
  lastNTPtime = time(&now);
  lastEntryTime = millis();

}

void loop() {
  getTimeReducedTraffic(3600);
  //getNTPtime(10);
  showTime(timeinfo);
  delay(1000);
}

void getTimeReducedTraffic(int sec) {
  tm *ptm;
  if ((millis() - lastEntryTime) < (1000 * sec)) {
    now = lastNTPtime + (int)(millis() - lastEntryTime) / 1000;
  } else {
    lastEntryTime = millis();
    lastNTPtime = time(&now);
    now = lastNTPtime;
    Serial.println("Get NTP time");
  }
  ptm = localtime(&now);
  timeinfo = *ptm;
  }


bool getNTPtime(int sec) {

  {
    uint32_t start = millis();
    do {
      time(&now);
      localtime_r(&now, &timeinfo);
      Serial.print(".");
      delay(10);
    } while (((millis() - start) <= (1000 * sec)) && (timeinfo.tm_year < (2016 - 1900)));
    
    if (timeinfo.tm_year <= (2016 - 1900)) return false;  // the NTP call was not successful
    Serial.print("now ");  Serial.println(now);
    char time_output[30];
    strftime(time_output, 30, "%a  %d-%m-%y %T", localtime(&now));
    Serial.println(time_output);
    Serial.println();
  
  }
  return true;
}

 text-align: center;
