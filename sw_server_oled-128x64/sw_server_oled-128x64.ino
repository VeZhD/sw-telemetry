/*********
Connect I2C Oled display 
SCL -> Lolin s2 mini - 35 / Lolin s3 mini - 36
SDA -> Lolin s2 mini - 33 / Lolin s3 mini - 35
VCC -> 5V
GND -> GND
*********/

// pins for Lolin s2 mini, for other boards - check and replace the pins with yours:
#define SENSOR_PIN 6     // пин подключения датчика луча

#define button01    13 // 38
#define button02    12 // 36
#define button03    10 // 34
#define HotPlug_pin 11

// #define button01    38
// #define button02    36
// #define button03    34

#if defined(ESP32)
  #pragma message "ESP32 stuff happening!"
#else
  #error "This ain't a ESP32\ESPs2\ESPs3\etc., dumbo!"
#endif

// pins for Lolin s3 mini and other boards on ESP32s3, check and replace the pins with yours:
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
#include <DNSServer.h>
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

// void StartStop_old() {
//   if (timerState == 0) {
//     if (startStopState == SensorState && startStopLastState == SensorLastState && millis() - lastChange > StopDelay) {
//       startTime = millis();

//       for (int i = LastTimeCount - 1; i > 0; i--) {
//         LastTime[i] = LastTime[i - 1];
//       }
//       LastTime[0] = currentTime;

//       timerState = 1;
//       currentTime = millis() - startTime;
//       lastChange = millis();
//       notifyClients();
//       }
//       startStopLastState = startStopState;
//     } else {
//       currentTime = millis() - startTime;
//       if (startStopState == SensorState && startStopLastState == SensorLastState && millis() - lastChange > StopDelay) {
//         timerState = 0;
//         lastChange = millis();
//         CalcTopTime();
//         notifyClients();
//         }
//       startStopLastState = startStopState;
//   }
// }

void StartStop() {
    if (startStopState == SensorState && startStopLastState == SensorLastState && millis() - lastChange > StopDelay) {
      if (timerState == 0) {
        startTime = millis();
        timerState = 1;
        notifyClients();
      } else {
        currentTime = millis() - startTime;
        for (int i = LastTimeCount - 1; i > 0; i--) {
          LastTime[i] = LastTime[i - 1];
        }
        LastTime[0] = currentTime;
        CalcTopTime();
        timerState = 0;
        notifyClients();
      }
      lastChange = millis();
    }
      startStopLastState = startStopState;
  if (timerState != 0) {
    currentTime = millis() - startTime;
  }
}

void TwoAthletes(byte athlet) {
 
  if (timerStateArray[athlet] == 0) {
    if (startStopState == SensorState && startStopLastState == SensorLastState && millis() - lastChange > StopDelay) {
      startTimeArray[athlet] = millis();
      for (int i = LastTimeCount - 1; i > 0; i--) {
        LastTime[i] = LastTime[i - 1];
      }
      LastTime[0] = currentTimeArray[athlet];

      timerStateArray[athlet] = 1;
      currentTimeArray[athlet] = millis() - startTimeArray[athlet];
      lastChange = millis();
      // notifyClients();
      athleteNumber = !athleteNumber;
      }
      startStopLastState = startStopState;
    } else {
      currentTimeArray[athlet] = millis() - startTimeArray[athlet];
      if (startStopState == SensorState && startStopLastState == SensorLastState && millis() - lastChange > StopDelay) {
        timerStateArray[athlet] = 0;
        lastChange = millis();
        // CalcTopTime();
        // notifyClients();
        if ( mode == "cr" ){
          athleteNumber = !athleteNumber;
        }
      }
      startStopLastState = startStopState;
  }
}

// void TwoAthletesLap() {

//     if (timerStateArray[athlet] == 0) {
//       if ( startStopState == SensorState && startStopLastState == SensorLastState && millis() - lastChange > StopDelay && athleteNumber == 0 ) {
//         startTime1 = millis();

//         for (int i = LastTimeCount - 1; i > 0; i--) {
//           LastTime[i] = LastTime[i - 1];
//         }
//         LastTime[0] = currentTime1;

//         timerState1 = 1;
//         athleteNumber = 1;

//         currentTime1 = millis() - startTime1;
//         lastChange = millis();
//         // notifyClients();
//         startStopLastState = startStopState;
//         }
        
//     } else {
//         currentTime1 = millis() - startTime1;
//         if ( startStopState == SensorState && startStopLastState == SensorLastState && millis() - lastChange > StopDelay && athleteNumber == 0 ) {
//           timerState1 = 0;
//           athleteNumber = 1;
//           lastChange = millis();
//           // CalcTopTime();
//           // notifyClients();
//           startStopLastState = startStopState;
//           }
//         }

//   startStopLastState = startStopState;

// }

void LapTimer() {
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
    if ( timerState == 0 ){ 
      timerState = 1;
      notifyClients();
    }
  }
}

void TwoAthletesLoop() {

  if (athleteNumber == 0) {
    if (startStopState == SensorState && startStopLastState == SensorLastState && millis() - lastChange > StopDelay) {
      currentTimeArray[1] = millis() - startTimeArray[1];
      startTimeArray[1] = millis();

      // CalcTopTime();

    // for (int i = LastTimeCount - 1; i > 0; i--) {
    //   LastTime[i] = LastTime[i - 1];
    // }
    // LastTime[0] = currentTime1;
  
    athleteNumber = 1;
    // notifyClients();
    lastChange = millis();
  }
  startStopLastState = startStopState;
    
    } else {
      if (startStopState == SensorState && startStopLastState == SensorLastState && millis() - lastChange > StopDelay) {
        currentTimeArray[2] = millis() - startTimeArray[2];
        startTimeArray[2] = millis();

      // CalcTopTime();

    // for (int i = LastTimeCount - 1; i > 0; i--) {
    //   LastTime[i] = LastTime[i - 1];
    // }
    // LastTime[0] = currentTime2;
  
        athleteNumber = 0;
    // notifyClients();
        lastChange = millis();
      }
      startStopLastState = startStopState;

    }

  if ( millis() - startTimeArray[1] >= PrintDelay && startTimeArray[1] !=0 ) {
    currentTimeArray[1] = millis() - startTimeArray[1];
  }
  if ( millis() - startTimeArray[2] >= PrintDelay && startTimeArray[2] !=0 ) {
    currentTimeArray[2] = millis() - startTimeArray[2];
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(HotPlug_pin, INPUT_PULLUP);

  pinMode(button01, INPUT_PULLUP);
  pinMode(button02, INPUT_PULLUP);
  pinMode(button03, INPUT_PULLUP);

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

  if ( mode == "ss"){ // Start-Stop timer
    StartStop();
  }
  else if ( mode == "lt" ) {  // Срабатывание таймера по каждому пересечению луча, как китайский лаптаймер
    LapTimer();
  }
  else if ( mode == "cr" ){
    TwoAthletes(athleteNumber);
    if ( timerStateArray[0] != 0 ){ currentTimeArray[0] = millis() - startTimeArray[0]; } 
    if ( timerStateArray[1] != 0 ){ currentTimeArray[1] = millis() - startTimeArray[1]; }       
  }
  else if ( mode == "2at" ){
    // TwoAthletesLap();
    TwoAthletes(athleteNumber);
    if ( timerStateArray[0] != 0 ){ currentTimeArray[0] = millis() - startTimeArray[0]; } 
    if ( timerStateArray[1] != 0 ){ currentTimeArray[1] = millis() - startTimeArray[1]; }    
  }
  else if ( mode == "2atl" ){
    TwoAthletesLoop();
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

  if ( mode == "cr"|| mode == "2at" || mode == "2atl" ){
    printtime2(currentTimeArray[0],currentTimeArray[1]);
  } else {
    printtime(currentTime);
    if (LastTimeID <10) {
      TimePrintXY(LastTime[LastTimeID], 0, 48, "LastTime 0" + String(LastTimeID) + ": ");
    }  else {
      TimePrintXY(LastTime[LastTimeID], 0, 48, "LastTime " + String(LastTimeID) + ": ");
    }

    TestPressButton01();
    TestPressButton02();
    TestPressButton03();
  }
  display.display();
}
