/*********
display LCD 1602 I2C with WEMOS(LOLIN) S2 mini
SCL -> D1(GPIO5) For ESP8266(Wemos D1 mini) / 35 For ESP32s2(Wemos(Lolin) S2 mini)
SDA -> D2(GPIO4) For ESP8266(Wemos D1 mini) / 33 For ESP32s2(Wemos(Lolin) S2 mini)
VCC -> 5V
GND -> GND
*********/
#define SSID_PIN      34    // пин кнопки переключения wifi сети
#define LAST_TIME_PIN 38    // пин кнопки переключения предыдущего времени(1-10)

#define EEPROM_SIZE   64    // размер EEPROM

// For ESP32\ESP32s2 and etc.
#include <WiFi.h>
// For ESP8266
//#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <EEPROM.h>

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#include "SSID_client.h"      // При необходимости изменить название и паролт WiFi точки доступа
#include "client.h"

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display


void printWifiState() {
  if (Connected == true) {
    lcd.setCursor(0,0);
    lcd.print("+");
  } else {
    lcd.setCursor(0,0);
    lcd.print("-");
  }
}

void printWsState() {
  if (ConnectedWS == true) {
    lcd.setCursor(2,0);
    lcd.print("+");
  } else {
    lcd.setCursor(2,0);
    lcd.print("-");
  }
}

void printSSID() {
  lcd.setCursor(0,1);
  lcd.print("Wifi:" + String(ssid[wifi_id]) + "     ");
}

void PrintTime() {
  lcd.setCursor(7,0);
  lcd.print(String(CurrentMinutes) + ":" + String(CurrenttSeconds) + String(Currentseconds) + "." + String(CurrentmSeconds) + String(CurrentmiSeconds) + String(CurrentmilSeconds) + "   ");
}

void TimePrintXY(uint32_t time, byte x, byte y, String name) {

  uint8_t minutes = (int)(time / 60000) % 10;
  uint8_t tSeconds = (int)(time % 60000 / 10000);
  uint8_t seconds = (int)(time % 60000 % 10000 / 1000);
  uint8_t mSeconds = (int)(time % 60000 % 1000 / 100);
  uint8_t miSeconds = (int)(time % 60000 % 100 / 10);
  uint8_t milSeconds = (int)(time % 60000 % 10);
  lcd.setCursor(x,y);
  lcd.print(name + String(minutes) + ":" + String(tSeconds) + String(seconds) + "." + String(mSeconds) + String(miSeconds) + String(milSeconds) + "    ");
}

void PrintCopyright(void) {
  lcd.setCursor(0,0);
  lcd.print("Made by VeZhD");
  lcd.setCursor(0,1);
  lcd.print("Code from Alekssaff");
  delay(750);
}

void LastTimeIDChangeLoop() {
  LastTimeID_State = digitalRead(LAST_TIME_PIN);
  if (LastTimeID_State == LOW && LastTimeID_LastState == HIGH) {
    if (LastTimeID >= 0 && LastTimeID < LastTimeCount) {
      LastTimeID++;
    } else {
      LastTimeID = 0;
    }
    LastTimeID_State_ts = millis();
  }
  LastTimeID_LastState = LastTimeID_State;
}

void setup() {
  pinMode(SSID_PIN, INPUT_PULLUP);
  pinMode(LAST_TIME_PIN, INPUT_PULLUP);  // пин кнопки переключения предыдущего времени(1-10)

  // For ESP8266
  //EEPROM.begin(EEPROM_SIZE);
  
  // For ESP32/ESP32s2
  if (!EEPROM.begin(EEPROM_SIZE)) {
    delay(1000000);
  }
  if (EEPROM.read(0) >= 0 && EEPROM.read(0) < (sizeof(ssid) / sizeof(char *))) {
    wifi_id = EEPROM.read(0);
  }

  WiFi.onEvent(WiFiEvent);

  lcd.init(); // initialize the lcd 
  lcd.backlight();

  // event handler
  webSocket.onEvent(webSocketEvent);
  PrintCopyright();
  connectToHost();
  lcd.clear();
}

void loop() {
  TimerLoop();
  ssidChangeLoop();
  LastTimeIDChangeLoop();
  webSocket.loop();
  printWifiState();
  printWsState();
  PrintTime();
 if (LastTimeID_State_ts < millis() - 15000) {
    LastTimeID = LastTimeCount;
  }
  if (ssid_state_ts > millis() - 5000) {
    printSSID();
  }
  else {
    if (LastTimeID == LastTimeCount) {
      TimePrintXY(TopTime, 0, 1, " Top: ");
    } 
    else {
      TimePrintXY(LastTime[LastTimeID], 0, 1, "Last" +  String(LastTimeID) + ": ");
    }
  }
  delay(1);
}
