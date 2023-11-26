#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <time.h>

// FreeFonts from Adafruit_GFX
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

///////////////////////////////////////////////////////////////////////////

#define RELEASE 0.5

#define DEBUG_SERIAL true      // Enable debbuging over serial interface
#if DEBUG_SERIAL
#define debug(x) Serial.print(x)
#define debugf(x, ...) Serial.printf((x), ##__VA_ARGS__)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugf(x, ...)
#define debugln(x)
#endif


#define TEMP 19                 // GPIO for OneWire-Bus
#define LED 13                  // GPIO for Alarm LED
#define BUZZER 10               // GPIO for Buzzer

#define BTN_TEMP_MINUS_TEN 12   // GPIO for TEMP -10
#define BTN_TEMP_MINUS 14       // GPIO for TEMP -1
#define BTN_TEMP_PLUS 27        // GPIO for TEMP +1
#define BTN_TEMP_PLUS_TEN 26    // GPIO for TEMP +10

#define BTN_TIME_MINUS_TEN 25   // GPIO for TIME -10
#define BTN_TIME_MINUS 33       // GPIO for TIME -1
#define BTN_TIME_PLUS 32        // GPIO for TIME +1
#define BTN_TIME_PLUS_TEN 35    // GPIO for TIME +10

#define BTN_PRESS_TIME 150      // Milliseconds Button pressed

// or select the display class and display driver class in the following file (new style):
#define ENABLE_GxEPD2_GFX 0
#define SCREEN_WIDTH   400 // 296
#define SCREEN_HEIGHT  300 // 128


// ======================================================================
// Setting parameters with default values
// ======================================================================

const char* WIFI_SSID = "WLAN";                      // WLAN-SSID
const char* WIFI_PW = "74696325262072177928";        // WLAN-Password
String HOSTNAME = "ESP-BrewController";              // Enter Hostname here
String MQTT_BROKER = "192.168.178.12";               // MQTT-Broker
String EXTERNAL_URL = "www.telekom.de";              // URL of external Website

String STATUS_MESSAGE = HOSTNAME;                    // Message in Display

const char* NTP_SERVER = "de.pool.ntp.org";
const char* TZ_INFO    = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";
tm timeinfo;
time_t now;

volatile boolean MANUAL_MODE = true;
hw_timer_t *Timer0_Cfg = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile int timerInterrupt = 0; 

// Encoder Button
portMUX_TYPE synch = portMUX_INITIALIZER_UNLOCKED;
volatile long lastButtonPressMS = 0;


int CURR_TEMP_1 = -11;
int CURR_TEMP_2 = -11;
int CURR_TEMP_3 = -11;
float TEMP_1 = 0;
float TEMP_2 = 0;
float TEMP_3 = 0;

volatile int TARGET_TEMP = 25;
volatile int TARGET_TEMP_MIN = 25;
volatile int TARGET_TEMP_MAX = 99;
int TEMP_HYSTERESIS = 2;
int CHART_VALUES [330];

volatile int TARGET_TIME = 30;
volatile int TARGET_TIME_MIN = 0;
volatile int TARGET_TIME_MAX = 120;

volatile boolean RUN_ONCE = true;
volatile boolean TIMER_STARTED = false;


int MINUTES = 0;                                    // Starttime minutes
int SECONDS = 0;                                    // Starttime seconds
char DATE[10];                                      // Set DATE
char TIME[7];                                       // Set TIME
String START_STAMP;                                 // Set Start Time
String REST_STAMP;                                  // Set Temp Reach Time
String END_STAMP;                                   // Set End Time

static TimerHandle_t refresh_display_timer = NULL;
static TimerHandle_t refresh_NTP_timer = NULL;
TaskHandle_t brewStep;

// ======================================================================
// Initialize Objects
// ======================================================================

// Initialize WebServer
AsyncWebServer server(80); 

// Initializing a oneWire instance to communicate with any OneWire devices
OneWire oneWire(TEMP);
DallasTemperature sensors(&oneWire);

// Connections for e.g. LOLIN D32
enum alignmentType {LEFT, RIGHT, CENTER};
static const uint8_t EPD_BUSY = 4;  // to EPD BUSY
static const uint8_t EPD_RST  = 16; // to EPD RST
static const uint8_t EPD_DC   = 17; // to EPD DC
static const uint8_t EPD_CS   = 5;  // to EPD CS
static const uint8_t EPD_SCK  = 18; // to EPD CLK
static const uint8_t EPD_MOSI = 23; // to EPD DIN
static const uint8_t EPD_MISO = 19; // Master-In Slave-Out not used, as no data from display

//GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT> display(GxEPD2_290_T94_V2(/*CS=D8*/ EPD_CS, /*DC=D3*/ EPD_DC, /*RST=D4*/ EPD_RST, /*BUSY=D2*/ EPD_BUSY));
GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display(GxEPD2_420(/*CS=D8*/ EPD_CS, /*DC=D3*/ EPD_DC, /*RST=D4*/ EPD_RST, /*BUSY=D2*/ EPD_BUSY));

SPIClass hspi(HSPI);

// ======================================================================
// Interrupt Routines
// ======================================================================

void IRAM_ATTR btnTempMinusTenISR() {
    portENTER_CRITICAL_ISR(&synch);
    if ((millis() - lastButtonPressMS) > BTN_PRESS_TIME) {
        lastButtonPressMS = millis();
        TARGET_TEMP = TARGET_TEMP - 10;
        if(TARGET_TEMP < TARGET_TEMP_MIN) { TARGET_TEMP = TARGET_TEMP_MAX; }

        START_STAMP = String(TIME);
        RUN_ONCE = true;
    }
    portEXIT_CRITICAL_ISR(&synch);
}

void IRAM_ATTR btnTempMinusISR() {
    portENTER_CRITICAL_ISR(&synch);
    if ((millis() - lastButtonPressMS) > BTN_PRESS_TIME) {
        lastButtonPressMS = millis();
        TARGET_TEMP--;
        if(TARGET_TEMP > TARGET_TEMP_MAX) { TARGET_TEMP = TARGET_TEMP_MAX; }
        
        START_STAMP = String(TIME);
        RUN_ONCE = true;
    }
    portEXIT_CRITICAL_ISR(&synch);
}

void IRAM_ATTR btnTempPlusISR() {
    portENTER_CRITICAL_ISR(&synch);
    if ((millis() - lastButtonPressMS) > BTN_PRESS_TIME) {
        lastButtonPressMS = millis();
        TARGET_TEMP++;
        if(TARGET_TEMP > TARGET_TEMP_MAX) { TARGET_TEMP = TARGET_TEMP_MAX; }

        START_STAMP = String(TIME);
        RUN_ONCE = true;
    }
    portEXIT_CRITICAL_ISR(&synch);
}

void IRAM_ATTR btnTempPlusTenISR() {   
    portENTER_CRITICAL_ISR(&synch);
    if ((millis() - lastButtonPressMS) > BTN_PRESS_TIME) {
        lastButtonPressMS = millis();
        TARGET_TEMP = TARGET_TEMP + 10;
        if(TARGET_TEMP > TARGET_TEMP_MAX) { TARGET_TEMP = TARGET_TEMP_MIN; }

        START_STAMP = String(TIME);
        RUN_ONCE = true;
    }
    portEXIT_CRITICAL_ISR(&synch);
}

void IRAM_ATTR btnTimeMinusTenISR() {
    portENTER_CRITICAL_ISR(&synch);
    if ((millis() - lastButtonPressMS) > BTN_PRESS_TIME) {
        lastButtonPressMS = millis();
        TARGET_TIME = TARGET_TIME - 10;
        RUN_ONCE = true;
        if(TARGET_TIME < TARGET_TIME_MIN) { TARGET_TIME = TARGET_TIME_MAX; }
    }
    portEXIT_CRITICAL_ISR(&synch);
}

void IRAM_ATTR btnTimeMinusISR() {
    portENTER_CRITICAL_ISR(&synch);
    if ((millis() - lastButtonPressMS) > BTN_PRESS_TIME) {
        lastButtonPressMS = millis();
        TARGET_TIME--;
        RUN_ONCE = true;
        if(TARGET_TIME < TARGET_TIME_MIN) { TARGET_TIME = TARGET_TIME_MIN; }
    }
    portEXIT_CRITICAL_ISR(&synch);
}

void IRAM_ATTR btnTimePlusISR() {
    portENTER_CRITICAL_ISR(&synch);
    if ((millis() - lastButtonPressMS) > BTN_PRESS_TIME) {
        lastButtonPressMS = millis();
        TARGET_TIME++;
        RUN_ONCE = true;
        if(TARGET_TIME > TARGET_TIME_MAX) { TARGET_TIME = TARGET_TIME_MAX; }
    }
    portEXIT_CRITICAL_ISR(&synch);
}

void IRAM_ATTR btnTimePlusTenISR() {   
    portENTER_CRITICAL_ISR(&synch);
    if ((millis() - lastButtonPressMS) > BTN_PRESS_TIME) {
        lastButtonPressMS = millis();
        TARGET_TIME = TARGET_TIME + 10;
        RUN_ONCE = true;
        if(TARGET_TIME > TARGET_TIME_MAX) { TARGET_TIME = TARGET_TIME_MIN; }
    }
    portEXIT_CRITICAL_ISR(&synch);
}


// ======================================================================
// Functions
// ======================================================================

void connectWifi();
void startWebServer();

void initRotaryEncoder();

void startBuzzer(String);
String processor(const String& var);
void loadConfig();
void saveConfig();
void saveConfig(AsyncWebServerRequest *request); 
void loadRecepy();

// Display related
void initDisplay();
void refreshDisplay(TimerHandle_t xTimer);
void refreshNTP(TimerHandle_t xTimer);
void partStatus();
void partTemp();
void partConfig();
void partChart();
void partProtocol();

// Brew Process
void brewStepsCode(void * pvParameters); 
void getTemp();

// ======================================================================
// Setup
// ======================================================================
void setup() {
    if(DEBUG_SERIAL){
        Serial.begin(115200);
        debug("Welcome to ESP-32!");
    }

    // Initialize CHART_VALUES array
    for (int i = 0; i < 329; i++){ CHART_VALUES[i] = 0; }

    // GPIO for LED
    pinMode(LED,OUTPUT);
    digitalWrite(LED, false);
    
    // GPIO for TEMP -10
    pinMode(BTN_TEMP_MINUS_TEN, INPUT_PULLUP);         
    attachInterrupt(BTN_TEMP_MINUS_TEN, btnTempMinusTenISR, FALLING);

    // GPIO for TEMP -1
    pinMode(BTN_TEMP_MINUS, INPUT_PULLUP);         
    attachInterrupt(BTN_TEMP_MINUS, btnTempMinusISR, FALLING);
    
    // GPIO for TEMP +1
    pinMode(BTN_TEMP_PLUS, INPUT_PULLUP);         
    attachInterrupt(BTN_TEMP_PLUS, btnTempPlusISR, FALLING);

    // GPIO for TEMP +10
    pinMode(BTN_TEMP_PLUS_TEN, INPUT_PULLUP);         
    attachInterrupt(BTN_TEMP_PLUS_TEN, btnTempPlusTenISR, FALLING);

    // GPIO for TIME -10
    pinMode(BTN_TIME_MINUS_TEN, INPUT_PULLUP);         
    attachInterrupt(BTN_TIME_MINUS_TEN, btnTimeMinusTenISR, FALLING);

    // GPIO for TIME -1
    pinMode(BTN_TIME_MINUS, INPUT_PULLUP);         
    attachInterrupt(BTN_TIME_MINUS, btnTimeMinusISR, FALLING);

    // GPIO for TIME +1
    pinMode(BTN_TIME_PLUS, INPUT_PULLUP);         
    attachInterrupt(BTN_TIME_PLUS, btnTimePlusISR, FALLING);

    // GPIO for TIME +10
    pinMode(BTN_TIME_PLUS_TEN, INPUT_PULLUP);         
    attachInterrupt(BTN_TIME_PLUS_TEN, btnTimePlusTenISR, FALLING);

    // init brew controller
    getTemp();
    loadConfig();
    connectWifi();
    startWebServer();

    configTime(0, 0, NTP_SERVER);
    setenv("TZ", TZ_INFO, 1);
    getLocalTime(&timeinfo);
    sprintf(TIME, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);

    debugf("TIME: %s\n", TIME);
    START_STAMP = String(TIME);
    initDisplay();

    //loadRecepy();

    // start update display timer
    refresh_display_timer = xTimerCreate(
                      "Refresh Temp on Display timer",  // Name of timer
                      1000 / portTICK_PERIOD_MS,        // Period of timer (in ticks)
                      pdTRUE,                           // Auto-reload
                      (void *)1,                        // Timer ID
                      refreshDisplay);                  // Callback function
    
    xTimerStart(refresh_display_timer, portMAX_DELAY);

    refresh_NTP_timer = xTimerCreate(
                      "Refresh NTP timer",              // Name of timer
                      60000 / portTICK_PERIOD_MS,       // Period of timer (in ticks)
                      pdTRUE,                           // Auto-reload
                      (void *)1,                        // Timer ID
                      refreshNTP);                      // Callback function

    xTimerStart(refresh_NTP_timer, portMAX_DELAY);

    xTaskCreatePinnedToCore(brewStepsCode, "Process Brew Steps", 2048, NULL, 2, &brewStep, 1);
}

// ======================================================================
// Loop
// ======================================================================
void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        digitalWrite(LED, true);        
    } else {
        digitalWrite(LED, false);
        connectWifi();
    }
    
    if (CHART_VALUES[328] > 0) {
        for (int i = 0; i < 329; i++) {
            CHART_VALUES[i] = CHART_VALUES[i + 1];
        }
        CHART_VALUES[329] = CURR_TEMP_1;
    } else {
        for (int i = 0; i < 329; i++) {
            if(CHART_VALUES[i] == 0) {
                CHART_VALUES[i] = CURR_TEMP_1;
                break;
            }
        }
    }  
    //STATUS_MESSAGE = String(timeinfo.tm_hour) + ":" + String(timeinfo.tm_min);
    //STATUS_MESSAGE = timeinfo.tm_hour + ":" + timeinfo.tm_min;
      
    delay(1000);
}

// ======================================================================
// Webserver related
// ======================================================================

void connectWifi() {
    //connect to your local wi-fi network
    //WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);

    if (WiFi.status() != WL_CONNECTED) {

        WiFi.setHostname(HOSTNAME.c_str());
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PW);
    
        debugf("Connecting to WiFi: %s\n", WIFI_SSID); 

        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            debug(".");
        }
        digitalWrite(LED, true);
        debugln("OK!");
        debugf("Hostname: %s\n", WiFi.getHostname());
        debugf("IP: %s\n", WiFi.localIP().toString().c_str());  
    }
}

void startWebServer(){
   
    ElegantOTA.begin(&server);    // Start ElegantOTA
    server.begin();
    debugln("HTTP server started!");

    
    // Make index.html available
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        debugln("Requested index.html page");
        request->send(LittleFS, "/index.html", String(), false, processor);
    });

    // Save config-parameters
    server.on("/save-targets", HTTP_GET, [](AsyncWebServerRequest *request){
        debugln("Save index.html page");
        saveConfig(request);
        request->redirect("/");
    });

    // Make config.html available
    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
        debugln("Requested config.html page");
        request->send(LittleFS, "/config.html", String(), false, processor);
    });

    // Make get data available
    server.on("/getdata", HTTP_GET, [](AsyncWebServerRequest *request){
              
        int paramsNr = request->params();
        for(int i=0;i<paramsNr;i++){
            AsyncWebParameter* p = request->getParam(i);
            
            if(p->name() == "TARGET_TEMP") {
                TARGET_TEMP = p->value().toInt();
            }
            if(p->name() == "TARGET_TIME" && !TIMER_STARTED) {
                TARGET_TIME = p->value().toInt();
            }
        }
        debugf("/getdata requested -> TARGET_TEMP:%d + TARGET_TIME:%d/n", TARGET_TEMP, TARGET_TIME);
        START_STAMP = String(TIME);
        RUN_ONCE = true;
        request->send(200, "text/plain", "Data received!");
    });

    // Save config-parameters
    server.on("/save-config", HTTP_GET, [](AsyncWebServerRequest *request){
        debugln("Save config.html page");
        saveConfig(request);
        request->redirect("/config");
    });

    // Make check.html available
    server.on("/check", HTTP_GET, [](AsyncWebServerRequest *request){
        debugln("Requested check.html page");
        request->send(LittleFS, "/check.html", String(), false, processor);
    });

    // Make recepy.html available
    server.on("/recepy", HTTP_GET, [](AsyncWebServerRequest *request){
        debugln("Requested recepy.html page");
        request->send(LittleFS, "/recepy.html", String(), false, processor);
    });

    // Make calc.html available
    server.on("/calc", HTTP_GET, [](AsyncWebServerRequest *request){
        debugln("Requested calc.html page");
        request->send(LittleFS, "/calc.html", String(), false, processor);
    });

    // Make style.css available
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/style.css","text/css");
    });

    // Make style.css available
    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
        debugln("Requested favicon.ico page");
        request->send(LittleFS, "/hop.ico","image");
    });


    server.on("/test", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Hi! This is ElegantOTA Demo.");
    });

}

String processor(const String& var){

    // index.html
    if(var == "MANUAL_MODE") {  
        if(MANUAL_MODE) {
            return String("MANUAL"); 
        } else {
            return String("AUTOMATIC"); 
        }
    }
    if(var == "CURR_TEMP_1"){  return String(CURR_TEMP_1);  }
    if(var == "CURR_TEMP_2"){  return String(CURR_TEMP_2);  }
    if(var == "CURR_TEMP_3"){  return String(CURR_TEMP_3);  } 
    if(var == "TIMER_SHOW"){  return String(MINUTES);  } 


    // config.html
    if(var == "HOSTNAME"){ return HOSTNAME; }
    if(var == "EXTERNAL_URL"){ return EXTERNAL_URL; } 
    if(var == "WIFI_SSID"){  return String(WIFI_SSID);  }
    if(var == "WIFI_PW"){  return String(WIFI_PW);  }
    if(var == "MQTT_BROKER"){  return String(MQTT_BROKER);  }
    if(var == "TARGET_TEMP"){  return String(TARGET_TEMP);  }
    if(var == "TARGET_TEMP_MIN"){  return String(TARGET_TEMP_MIN);  }
    if(var == "TARGET_TEMP_MAX"){  return String(TARGET_TEMP_MAX);  }
    if(var == "TEMP_HYSTERESIS"){  return String(TEMP_HYSTERESIS);  }
    if(var == "TARGET_TIME"){  return String(TARGET_TIME);  }
    if(var == "TARGET_TIME_MIN"){  return String(TARGET_TIME_MIN);  }
    if(var == "TARGET_TIME_MAX"){  return String(TARGET_TIME_MAX);  }
    debugf("processor -> TARGET_TEMP:%d TARGET_TIME:%d TEMP_HYSTERESIS:%d\n", TARGET_TEMP, TARGET_TIME, TEMP_HYSTERESIS);

    // Footer
    if(var == "RELEASE"){  return String(RELEASE);  } 

    return String();   
}

void loadConfig(){
    if (!LittleFS.begin(true)) { 
        debugln("! An error occurred during SPIFFS mounting !");
        return; 
    }

    File config = LittleFS.open("/config.json","r");

    if(config) { 
        // Deserialize the JSON document
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, config);

        debugln(config.readString());

        if (error) { 
            debugf("! DeserializeJson failed! -> %s\n", error.f_str());
        }
        JsonObject obj = doc.as<JsonObject>();
                
        String hostname = obj["HOSTNAME"];
        HOSTNAME = hostname.c_str();
        String mqtt_broker = doc["MQTT_BROKER"];     
        MQTT_BROKER = mqtt_broker.c_str();
        String ext_url = doc["EXTERNAL_URL"];     
        EXTERNAL_URL = ext_url.c_str();
        TARGET_TEMP = doc["TARGET_TEMP"];
        TARGET_TEMP_MIN = doc["TARGET_TEMP_MIN"];
        TARGET_TEMP_MAX = doc["TARGET_TEMP_MAX"];
        TEMP_HYSTERESIS = doc["TEMP_HYSTERESIS"];
        TARGET_TIME = doc["TARGET_TIME"];
        TARGET_TIME_MIN = doc["TARGET_TIME_MIN"];
        TARGET_TIME_MAX = doc["TARGET_TIME_MAX"];
    }
    config.close();
}

void saveConfig(AsyncWebServerRequest *request) {
    if (!LittleFS.begin(true)) { 
        debugln("!An error occurred during LittleFS mounting!");
        return; 
    }

    File config = LittleFS.open("/config.json","w");

    if(config) { 
        // Serialize the JSON document
        DynamicJsonDocument doc(1024);

        int paramsNr = request->params();
        for(int i=0;i<paramsNr;i++){
            AsyncWebParameter* p = request->getParam(i);
            
            if(p->name() == "HOSTNAME") {
                HOSTNAME = p->value();
                doc["HOSTNAME"] = HOSTNAME;
            }

            if(p->name() == "MQTT_BROKER") {
                MQTT_BROKER = p->value();
                doc["MQTT_BROKER"] = MQTT_BROKER;
            }

            if(p->name() == "EXTERNAL_URL") {
                EXTERNAL_URL = p->value();
                doc["EXTERNAL_URL"] = EXTERNAL_URL;
            }

            if(p->name() == "TEMP_HYSTERESIS") {
                TEMP_HYSTERESIS = p->value().toInt();
                doc["TEMP_HYSTERESIS"] = TEMP_HYSTERESIS;
            }

            if(p->name() == "TARGET_TEMP") {
                TARGET_TEMP = p->value().toInt();
                doc["TARGET_TEMP"] = TARGET_TEMP;
            }

            if(p->name() == "TARGET_TEMP_MIN") {
                TARGET_TEMP_MIN = p->value().toInt();
                doc["TARGET_TEMP_MIN"] = TARGET_TEMP_MIN;
            }

            if(p->name() == "TARGET_TEMP_MAX") {
                TARGET_TEMP_MAX = p->value().toInt();
                doc["TARGET_TEMP_MAX"] = TARGET_TEMP_MAX;
            }

            if(p->name() == "TARGET_TIME") {
                TARGET_TIME = p->value().toInt();
                doc["TARGET_TIME"] = TARGET_TIME;
            }

            if(p->name() == "TARGET_TIME_MIN") {
                TARGET_TIME_MIN = p->value().toInt();
                doc["TARGET_TIME_MIN"] = TARGET_TIME_MIN;
            }

            if(p->name() == "TARGET_TIME_MAX") {
                TARGET_TIME_MAX = p->value().toInt();
                doc["TARGET_TIME_MAX"] = TARGET_TIME_MAX;
            }
        }
    
        serializeJsonPretty(doc, config);
    }
    config.close();
}

void saveConfig() {
    if (!LittleFS.begin(true)) { 
        debugln("!An error occurred during LittleFS mounting!");
        return; 
    }

    File config = LittleFS.open("/config.json","w");

    if(config) { 
        // Serialize the JSON document
        DynamicJsonDocument doc(1024);
        doc["HOSTNAME"] = HOSTNAME;
        doc["MQTT_BROKER"] = MQTT_BROKER;
        doc["EXTERNAL_URL"] = EXTERNAL_URL;
        doc["TEMP_HYSTERESIS"] = TEMP_HYSTERESIS;
        doc["TARGET_TEMP"] = TARGET_TEMP;
        doc["TARGET_TEMP_MIN"] = TARGET_TEMP_MIN;
        doc["TARGET_TEMP_MAX"] = TARGET_TEMP_MAX;
        doc["TARGET_TIME"] = TARGET_TIME;     
        doc["TARGET_TIME_MIN"] = TARGET_TIME_MIN;    
        doc["TARGET_TIME_MAX"] = TARGET_TIME_MAX;        
        serializeJsonPretty(doc, config);
    }   
    config.close();
}

void loadRecepy(){
    if (!LittleFS.begin(true)) { 
        debugln("! An error occurred during SPIFFS mounting !");
        return; 
    }

    File recepy = LittleFS.open("/recepy.json","r");

    if(recepy) { 
        // Deserialize the JSON document
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, recepy);

        debugln(recepy.readString());
    }
    recepy.close();
}

// ======================================================================
// e-Paper Display related
// ======================================================================

void initDisplay(){

    if (DEBUG_SERIAL) { display.init(115200, true, 2, false); }
    display.setPartialWindow(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    display.setRotation(0);                    // Use 1 or 3 for landscape modes

    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.firstPage();

    do
    {
        partStatus();   // Status Bar
        partTemp();     // Temp
        partConfig();   // Config
        partChart();    // Chart
    }
    while (display.nextPage());
}

void refreshDisplay(TimerHandle_t xTimer){
    
    //debugln("updateTempTimer expired!");
    getTemp();
    int x = 0;
    int y = 0;
    int h = SCREEN_HEIGHT;
    int w = SCREEN_WIDTH;
    display.setPartialWindow(x,y,w,h);
    display.firstPage();

    do
    {
        partStatus();   // Status Bar
        partTemp();     // Temp
        partConfig();   // Config
        partChart();    // Chart
    } 
    while (display.nextPage());
}

void partStatus(){
    // Status Bar
    display.drawLine(0,16,SCREEN_WIDTH,16,GxEPD_BLACK);
    //display.drawLine(SCREEN_WIDTH/2,16,SCREEN_WIDTH/2,SCREEN_HEIGHT, GxEPD_BLACK);
    display.setFont(&FreeMono9pt7b);

    getLocalTime(&timeinfo);
    sprintf(TIME,"%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    sprintf(DATE,"%02d.%02d.%04d", timeinfo.tm_mday, timeinfo.tm_mon+1, timeinfo.tm_year+1900);
    
    display.setCursor(0, 11);
    display.print(DATE);

    display.setCursor(120, 11);
    display.print(TIME);
    
    display.setCursor(190, 11);
    String RSSI_percent = String((100 + WiFi.RSSI())*2);
    display.print("RSSI:" + String(WiFi.RSSI())+"dBm(" + RSSI_percent +"%)");
    //display.print("Release:" + String(RELEASE));


    if(WiFi.status() == WL_CONNECTED) {
        display.fillRect(381, 9, 3, 4, GxEPD_BLACK);
        display.fillRect(385, 6, 3, 7, GxEPD_BLACK);
        display.fillRect(389, 3, 3, 10, GxEPD_BLACK);
        display.fillRect(393, 0, 3, 13, GxEPD_BLACK);
    } else {
        display.fillRect(381,0,396,13, GxEPD_WHITE);
    }
}

void partTemp(){

    int x = 0;
    int y = 16;
    int h = 150;
    int w = SCREEN_WIDTH/2;    
    //display.drawRect(x,y,w,h,GxEPD_BLACK);

    // Temp 1
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(0, 47);
    display.print("Temp1:");
    display.setFont(&FreeMonoBold24pt7b);
    display.setCursor(80, 55);
    display.printf("%3dc", CURR_TEMP_1);
    display.drawCircle(166, 34, 3, GxEPD_BLACK);
       
    // Time
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(0, 79);
    display.print("Time:");
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(70, 85);
    display.printf("%3d:%02d", MINUTES, SECONDS);

    // Temp 2
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(0, 122);
    display.print("Temp2:");
    display.setFont(&FreeMonoBold24pt7b);
    display.setCursor(80, 130);
    display.printf("%3dc", CURR_TEMP_2);
    display.drawCircle(166, 109, 3, GxEPD_BLACK);

    // Temp 3
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(0, 162);
    display.print("Temp3:");
    display.setFont(&FreeMonoBold24pt7b);
    display.setCursor(80, 170);
    display.printf("%3dc", CURR_TEMP_3);
    display.drawCircle(166, 149, 3, GxEPD_BLACK);
}

void partConfig() {
    int x = SCREEN_WIDTH/2;
    int y = 16;
    int h = 150;
    int w = SCREEN_WIDTH/2;
    int offset = 20;
    //display.drawRect(x,y,w,h,GxEPD_BLACK);
  
    // Config
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(x+offset, 35);
    display.println("Settings:");      
        
    // TEMP
    display.setCursor(x+offset, 55);
    display.setFont(&FreeMono9pt7b);  
    display.println("Target Temp:");
    display.setCursor(x+offset+145, 55);
    display.printf("%3d", TARGET_TEMP);

    // TIME
    display.setCursor(x+offset, 75); 
    display.setFont(&FreeMono9pt7b);
    display.println("Target Time:");
    display.setCursor(x+offset+145, 75);
    display.printf("%3d", TARGET_TIME);

    // Protocol
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(x+offset, 110);
    display.println("Protocol:");    

    // Start Time
    display.setCursor(x+offset, 130); 
    display.setFont(&FreeMono9pt7b);
    display.println("Start Time:");
    display.setCursor(x+offset+125, 130);
    display.print(START_STAMP);

    // Rest Time
    display.setCursor(x+offset, 150); 
    display.setFont(&FreeMono9pt7b);
    display.println("Rest Time:");
    display.setCursor(x+offset+125, 150);
    display.print(REST_STAMP);

    // Rest Time
    display.setCursor(x+offset, 170); 
    display.setFont(&FreeMono9pt7b);
    display.println("End Time:");
    display.setCursor(x+offset+125, 170);
    display.print(END_STAMP);
}

void partChart(){
    int x = 0;
    int y = 166;
    int h = SCREEN_HEIGHT - y;
    int w = SCREEN_WIDTH;    
    
    //display.drawRect(x,y,w,h,GxEPD_BLACK);
    display.drawFastHLine(40, 290, 340, GxEPD_BLACK);
    display.drawLine(380,290,375,285, GxEPD_BLACK);
    display.drawLine(380,290,375,295, GxEPD_BLACK);

    display.drawFastVLine(40, 180, 110, GxEPD_BLACK);
    display.drawLine(40,180,35,185, GxEPD_BLACK);
    display.drawLine(40,180,45,185, GxEPD_BLACK);

    display.setFont(&FreeMono9pt7b);
    for(int i = 0 ; i < 101; i = i + 20){
        display.drawFastHLine(35,290-i,10,GxEPD_BLACK);
        display.setCursor(2,294-i);
        display.printf("%3d", i);
    }

    for(int i = 40 ; i < 360; i = i + 5){
        display.drawPixel(i,290-TARGET_TEMP,GxEPD_BLACK);
    }
    display.setCursor(360,290-TARGET_TEMP);
    display.printf("%dc", TARGET_TEMP);

    for(int i = 0; i < 329; i++){
        display.drawPixel(i+40,290-CHART_VALUES[i],GxEPD_BLACK);
    }

    //display.drawFastHLine(40,280-TARGET_TEMP,360,GxEPD_BLACK);
}

void partProtocol(){

}

// ======================================================================
// Brew Process related
// ======================================================================

void getTemp() {
    sensors.requestTemperatures();
    int temp_hysteris = 9;
    float TEMP_1 = sensors.getTempCByIndex(0);
    //if (temp_fridge > 0) { CURR_TEMP_F = temp_fridge; }
    if (CURR_TEMP_1 == -11 || TEMP_1 > CURR_TEMP_1-temp_hysteris && TEMP_1 < CURR_TEMP_1+temp_hysteris)
    { CURR_TEMP_1=(int)round(TEMP_1);}

    float TEMP_2 = sensors.getTempCByIndex(1);
    //if (temp_fridge > 0) { CURR_TEMP_F = temp_fridge; }
    if (CURR_TEMP_2 == -11 || TEMP_2 > CURR_TEMP_2-temp_hysteris && TEMP_2 < CURR_TEMP_2+temp_hysteris)
    { CURR_TEMP_2= (int)round(TEMP_2);}

    float TEMP_3 = sensors.getTempCByIndex(2);
    //if (temp_fridge > 0) { CURR_TEMP_F = temp_fridge; }
    if (CURR_TEMP_3 == -11 || TEMP_3 > CURR_TEMP_3-temp_hysteris && TEMP_3 < CURR_TEMP_3+temp_hysteris)
    { CURR_TEMP_3=(int)round(TEMP_3);}
}

void brewStepsCode(void * pvParameters) {
    debugf("brewStepsCode on CPU: %i\n", xPortGetCoreID());
    for(;;) {
        // Start monitoring the temperature und end monitoring and altering when target temp is reached.
        if((CURR_TEMP_1 > TARGET_TEMP) && !TIMER_STARTED && RUN_ONCE) {
            if (TARGET_TIME > 0) {
                TIMER_STARTED = true;
                MINUTES = TARGET_TIME;
                REST_STAMP = String(TIME);
            } 
            
            tone(BUZZER, 4000, 1000);
            debugf("CURR_TEMP_1 reached: %02d\n", CURR_TEMP_1);  
        }      
    
        // Give an alert, when timer-modus is started and temperature is under target temperature
        if(CURR_TEMP_1 < TARGET_TEMP - TEMP_HYSTERESIS && TIMER_STARTED) {
            tone(BUZZER, 6000, 500);
            debugf("CURR_TEMP_1 under: %02d\n", (TARGET_TEMP - TEMP_HYSTERESIS));
        }

        // If target temp is reached, start the count down
        if(TIMER_STARTED && RUN_ONCE) {
            unsigned long previousMillis = 0;
            const long interval = 1000; // Intervall in Millisekunden (1 Sekunde)

            unsigned long currentMillis = millis();

            if (currentMillis - previousMillis >= interval) {
                previousMillis = currentMillis;
                
                // Verringere die Sekunden
                SECONDS--;

                // Überprüfe, ob die Sekunden auf 0 sind und verringere die Minuten entsprechend
                if (SECONDS < 0) {
                    SECONDS = 59;
                    MINUTES--;
                }

                // Überprüfe, ob die Zeit abgelaufen ist
                if (MINUTES == 0 && SECONDS == 0 ) {
                    debugln("Zeit abgelaufen!");
                    END_STAMP = String(TIME);
                    TIMER_STARTED = false;
                    RUN_ONCE = false;
                    tone(BUZZER, 4000, 300);
                    tone(BUZZER, 3000, 300);
                    tone(BUZZER, 4000, 300);
                    tone(BUZZER, 3000, 300);
                    tone(BUZZER, 4000, 300);
                }
                                
                //debugf("Verbleibende Zeit: %03d:%02d\n", MINUTES, SECONDS); // Anzeige der verbleibenden Minuten und Sekunden
            }
        }
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


// ======================================================================
// Rest
// ======================================================================

void refreshNTP(TimerHandle_t xTimer) {


    getLocalTime(&timeinfo);
    STATUS_MESSAGE = timeinfo.tm_hour + ":" + timeinfo.tm_min;
    debugf("TIME: %s", STATUS_MESSAGE);

}

