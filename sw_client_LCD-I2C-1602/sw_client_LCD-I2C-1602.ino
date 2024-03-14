/*********
display LCD 1602 I2C with WEMOS(LOLIN) S2 mini
SCL -> D1(GPIO5) For ESP8266(Wemos D1 mini) / 35 For ESP32s2(Wemos(Lolin) S2 mini)
SDA -> D2(GPIO4) For ESP8266(Wemos D1 mini) / 33 For ESP32s2(Wemos(Lolin) S2 mini)
VCC -> 5V
GND -> GND
*********/
#define DISPLAY_LASTTIME 0
#define SW_Basic_OTA_HOSTNAME SWC_LCD1602  // HostName для ESP
//#define SW_Basic_OTA_PASSWORD passwordSWC_LCD1602  // пароль для OTA обновления, по умолчанию "passwordSW_client", без ковычек

#define SSID_PIN      34    // пин кнопки переключения wifi сети
#define LAST_TIME_PIN 38    // пин кнопки переключения предыдущего времени(1-10)

#define EEPROM_SIZE   64    // размер EEPROM

// For ESP32\ESP32s2 and etc.
#include <WiFi.h>
// For ESP8266
//#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>

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

  SW_Basic_OTA();
}

void loop() {
  ArduinoOTA.handle();

  TimerLoop();
  ssidChangeLoop();
  LastTimeIDChangeLoop();
  webSocket.loop();
  printWifiState();
  printWsState();
  PrintTime();

  if (ssid_state_ts + 5000 > millis()) {
    printSSID();
  }
  else {
    if (LastTimeID == 0) {
      TimePrintXY(TopTime, 0, 1, " Top: ");
    } 
    else {
      TimePrintXY(LastTime[LastTimeID - 1], 0, 1, "Last" +  String(LastTimeID - 1) + ": ");
    }
  }
  
  delay(1);
}
