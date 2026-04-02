/*********
Connect I2C Oled display 
SCL -> Lolin s2 mini - 35 / Lolin s3 mini - 36
SDA -> Lolin s2 mini - 33 / Lolin s3 mini - 35
VCC -> 5V
GND -> GND
*********/
// #define OLD_index_html 

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Update.h>


// Последовательность подключения следующих файлов важна, не менять местами:
#include "vars.h"
#include "sw_config.h"
#include "timer.h"
#include "sw_server.h"
#include "128x64.h"
#include "routes.h"

void StartStop() {
    if ( millis() - lastChange >= StopDelay ) {
      if (timerState == 0) {
        startTime = millis();
        timerState = 1;
        currentTime = millis() - startTime;
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
}

void LapTimer() {
  if ( millis() - lastChange > StopDelay ) {
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
}

void TwoAthletes(byte athlet) {
 
  if (timerStateArray[athlet] == 0) {
    if ( millis() - lastChange > StopDelay ) {
      startTimeArray[athlet] = millis();
      for (int i = LastTimeCount - 1; i > 0; i--) {
        LastTime[i] = LastTime[i - 1];
      }
      LastTime[0] = currentTimeArray[athlet];

      timerStateArray[athlet] = 1;
      currentTimeArray[athlet] = millis() - startTimeArray[athlet];
      lastChange = millis();

      athleteNumber = !athleteNumber;
      }
    } else {
      currentTimeArray[athlet] = millis() - startTimeArray[athlet];
      if ( millis() - lastChange > StopDelay ) {
        timerStateArray[athlet] = 0;
        lastChange = millis();
        // CalcTopTime();
        currentTime = currentTimeArray[athlet];
        timerState = 0;
        notifyClients();
        if ( mode == dss ){
          athleteNumber = !athleteNumber;
        }
      }
  }
}

void TwoAthletesLoop() {

    if ( millis() - lastChange > StopDelay ) {
      currentTimeArray[athleteNumber] = millis() - startTimeArray[athleteNumber];
      startTimeArray[athleteNumber] = millis();

      // CalcTopTime();

    // for (int i = LastTimeCount - 1; i > 0; i--) {
    //   LastTime[i] = LastTime[i - 1];
    // }
    // LastTime[0] = currentTime1;
  
    currentTime = currentTimeArray[athleteNumber];
    timerState = 0;
    notifyClients();

    athleteNumber = !athleteNumber;
    lastChange = millis();
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(HotPlug_pin, INPUT_PULLUP);

  pinMode(button01, INPUT_PULLUP);
  pinMode(button02, INPUT_PULLUP);
  pinMode(button03, INPUT_PULLUP);

  InitConfig();

  initDisplay();

  initWifi( digitalRead(button03) );

  InitDNS();

  printip();

  initWebSocket();

  InitRoutes();

  server.begin();
}

void loop() {

  switch (mode) {
    case ss: {
      if ( timerState == 1 ){ currentTime = millis() - startTime; } 
      break;
    }

    case lt: {
      if (millis() - startTime >= PrintDelay && LastTime[0] != 0) {
        currentTime = millis() - startTime;
        if ( timerState == 0 ){ 
          timerState = 1;
          notifyClients();
        }
      }
      break;
    }

    case dss: 
    case sss: {
      for (byte i = 0; i < numberOfAthlete; i++){
        if ( timerStateArray[i] != 0 ){ currentTimeArray[i] = millis() - startTimeArray[i]; }
      }
      break;
    }

    case dlt: {
      if ( millis() - startTimeArray[0] >= PrintDelay && startTimeArray[0] !=0 ) {
        currentTimeArray[0] = millis() - startTimeArray[0];
      }
      if ( millis() - startTimeArray[1] >= PrintDelay && startTimeArray[1] !=0 ) {
        currentTimeArray[1] = millis() - startTimeArray[1];
      }
      break;
    }
  }

  dnsServer.processNextRequest();

  laptime = startStopStateName + String(timerState) + String(currentTime);

  ChangeModeLoop();
  ssidChangeLoop();

  LastTimeIDChangeLoop();
  FontChangeLoop();

  if ( hotPlug_display() ) {

    if ( mode == dss || mode == sss || mode == dlt ){
      printtime2(currentTimeArray[0],currentTimeArray[1]);
    } else {
      printtime(currentTime);
      TimePrintXY(LastTime[LastTimeID], 0, 48, ((LastTimeID <10) ? "LastTime 0" : "LastTime ") + String(LastTimeID) + ": ");

      TestPressButton();
    }

    display.display();
  }

}
