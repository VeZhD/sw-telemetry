/*********
Connect I2C Oled display 
SCL -> Lolin s2 mini - 35 / Lolin s3 mini - 36
SDA -> Lolin s2 mini - 33 / Lolin s3 mini - 35
VCC -> 5V
GND -> GND
*********/

//#define SENSOR_NPN // при использовании сенсора с NPN укзать SENSOR_NPN, при использовании сенсора с PNP укзать SENSOR_PNP
//#define SENSOR_NO // при использовании сенсора с NO(Normal Open, нормально открытый) укзать SENSOR_NO, при использовании сенсора NC(Normal Closed, нормально закрытый) укзать SENSOR_NC

// pins for Lolin s2 mini, for other boards, check and replace the pins with yours:
#define SENSOR_PIN 6     // пин подключения датчика луча

#define button01    13
#define button02    12
#define button03    10
#define HotPlug_pin 11

//#define EEPROM_SIZE   64    // размер EEPROM

#if defined(ESP32)
  #pragma message "ESP32 stuff happening!"
#else
  #error "This ain't a ESP32\ESPs2\ESPs3\etc., dumbo!"
#endif

// pins for Lolin s3 mini
#ifdef CONFIG_IDF_TARGET_ESP32S3

#undef SENSOR_PIN     // пин подключения датчика луча
#undef button01
#undef button02
#undef button03
#undef HotPlug_pin

#define SENSOR_PIN    6     // пин подключения датчика луча

#define button01      9
#define button02      10
#define button03      8
#define HotPlug_pin   11

#endif

bool  button01_State;
bool  button01_LastState = HIGH;

bool  button02_State;
bool  button02_LastState = HIGH;

bool  button03_State;
bool  button03_LastState = HIGH;

#include <WiFi.h>
#include <AsyncTCP.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
//#include <AsyncElegantOTA.h>
#include <DNSServer.h>
// #include <EEPROM.h>
#include <Update.h>
#include <SPIFFS.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

// Последовательность подключения следующих файлов важна, не менять местами:
#include "config.h"
#include "timer.h"
#include "server.h"
#include "128x64.h"
#include "ota_update.h"

#include "html_get.h"
#include "favicon.h"
#include "script.js.gz.h"
#include "NoSleep.min.js.gz.h"

#include "routes.h"

/*
boolean find_i2c(uint8_t address) { // функция проверки устройства по указанному адресу I2C
  Wire.beginTransmission(address);
  return (Wire.endTransmission () == 0); //возвращает true если ошибок нет (устройство подключено и отвечает)
}
*/

void setup() {
  Serial.begin(115200);

  pinMode(HotPlug_pin, INPUT_PULLUP);

  pinMode(button01, INPUT_PULLUP);
  pinMode(button02, INPUT_PULLUP);
  pinMode(button03, INPUT_PULLUP);


  // #if defined(ESP32)
  //   // For ESP32/ESP32s2/ESP32s3
  //   if (!EEPROM.begin(EEPROM_SIZE)) {
  //     delay(1000000);
  //   } 
  // #elif defined(ESP8266)
  //   // For ESP8266
  //   EEPROM.begin(EEPROM_SIZE);
  // #endif

  InitConfig();

  InitDisplay();

  InitWifi();

  InitDNS();

  printip();

  initWebSocket();

  InitRoutes();

  OTAWeb_update_begin();

  server.begin();
}

void loop() {
  startStopState = digitalRead(SENSOR_PIN);
  
  // if ( ( sensorType == "npn" and gate == "no" ) or ( sensorType == "pnp" and gate == "nc" ) ) {
  //   startStopState = !digitalRead(SENSOR_PIN);
  //   // SensorState = LOW;
  //   // SensorLastState = HIGH;
  // } else if ( ( sensorType == "pnp" and gate == "no" ) or ( sensorType == "npn" and gate == "nc" ) ) {
  //   startStopState = digitalRead(SENSOR_PIN);
  //   // SensorState = HIGH;
  //   // SensorLastState = LOW;
  // }

  if ( mode == "ss"){ // Start-Stop timer
      if (timerState == 0) {
        if (startStopState == SensorState && startStopLastState == SensorLastState && millis() - lastChange > StopDelay) {
          startTime = millis();

          for (int i = LastTimeCount - 1; i > 0; i--) {
            LastTime[i] = LastTime[i - 1];
          }
          LastTime[0] = currentTime;

          timerState = 1;
          currentTime = millis() - startTime;
          lastChange = millis();
          notifyClients();
        }
        startStopLastState = startStopState;
      } else {
        currentTime = millis() - startTime;
        if (startStopState == SensorState && startStopLastState == SensorLastState && millis() - lastChange > StopDelay) {
          timerState = 0;
          lastChange = millis();
          CalcTopTime();
          notifyClients();
          }
        startStopLastState = startStopState;
      }
  }
  else if ( mode == "lt" ) {  // Срабатывание таймера по каждому пересечению луча, как китайский лаптаймер
    if (startStopState == SensorState && startStopLastState == SensorLastState && millis() - lastChange > StopDelay) {
        currentTime = millis() - startTime;
        startTime = millis();

        CalcTopTime();

        for (int i = LastTimeCount - 1; i > 0; i--) {
          LastTime[i] = LastTime[i - 1];
        }
        LastTime[0] = currentTime;

        timerState = 0;
        notifyClients();
        lastChange = millis();
      }
      startStopLastState = startStopState;
    
      if (millis() - startTime >= PrintDelay) {
        currentTime = millis() - startTime;
        timerState = 1;
        notifyClients();
      }
  }

  dnsServer.processNextRequest();

  laptime = startStopStateName + String(timerState) + String(currentTime);

  //if (millis() - 1555 > lastWsUpTime) {
  //  notifyClients();
  //}  
  HotPlug_display();

  ChangeModeLoop();
  ssidChangeLoop();

  LastTimeIDChangeLoop();
  FontChangeLoop();

  printtime(currentTime);
  if (LastTimeID <10) {
    TimePrintXY(LastTime[LastTimeID], 0, 48, "LastTime 0" + String(LastTimeID) + ": ");
    }
  else {
    TimePrintXY(LastTime[LastTimeID], 0, 48, "LastTime " + String(LastTimeID) + ": ");
  }

  TestPressButton01();
  TestPressButton02();
  TestPressButton03();
  
  display.display();
}
