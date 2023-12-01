# ESP Brew Controller: Change Log


## ToDo: Next Release (0.7):
https://github.com/wemos/D1_mini_Examples/blob/master/examples/02.Special/CheckFlashConfig/CheckFlashConfig.ino
https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/FreeRTOS/BasicMultiThreading/BasicMultiThreading.ino
vTaskDelete(NULL); 



## Release (0.6):
- add code for a rtos related wifi control

WIFI_TIMEOUT_MS 200000

void keepWiFiAlive(void * parameters) {
  for(;;){
    if(WiFi.status() == WL_CONNECTED) {
    vTaskDelay(10000 / portTICK_PERIOD_MS):
    continue;  
    }

    // try to connect WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_NETWORK, WIFI_PW);
    unsigned long startAttempTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis()-startAttempTime < WIFI_TIMEOUT_MS){}
   
    if(WiFi.status() != WL_CONNECTED) { 
      WiFi failed;
      vTaskDelay(20000 / portTICK_PERIOD_MS):
      continue;
     }

  WiFi connected!!
} 

void setup() {
  xTaskCreateToPinnedToCore(
    keepWiFiAlive,
    "Keeping WiFi Alive",
    2024;
    NULL,
    1,
    NULL,
    0
  );
}


## Next Release (0.5):
- First runable Version with NTP Timesync, RSSI-Value, Chart, Protocol, Buzzer and brew process control. 
