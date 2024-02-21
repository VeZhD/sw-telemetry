/*********
display OLED i2c 128*64 witch WEMOS(LOLIN) S2 mini
SCL -> D1(GPIO5) For ESP8266(Wemos D1 mini) / 35 For ESP32s2(Wemos(Lolin) S2 mini)
SDA -> D2(GPIO4) For ESP8266(Wemos D1 mini) / 33 For ESP32s2(Wemos(Lolin) S2 mini)
VCC -> 5V
GND -> GND
*********/

#if defined(ESP32)
  #pragma message "ESP32 stuff happening!"
  #define SDA_PIN 33 //change for yours SDA pin on ESP32\32s2\etc.
  #define SCL_PIN 35 //change for yours SCL pin on ESP32\32s2\etc.

  #define SSID_PIN      34   // пин кнопки переключения wifi сети
  #define FONT_PIN      36   // пин кнопки переключения шрифта
  #define LAST_TIME_PIN 38   // пин кнопки переключения предыдущего времени(1-10)

  #include <WiFi.h>

#elif defined(ESP8266)
  #pragma message "ESP8266 stuff happening!"
  #define SDA_PIN 4 // change for yours SDA pin on ESP8266
  #define SCL_PIN 5 // changr for yours SCL pin on ESP8266

  #define SSID_PIN      1   // пин кнопки переключения wifi сети
  #define FONT_PIN      2   // пин кнопки переключения шрифта
  #define LAST_TIME_PIN 3   // пин кнопки переключения предыдущего времени(1-10)
  
  #include <ESP8266WiFi.h>

#else
#error "This ain't a ESP8266 or ESP32, dumbo!"
#endif

#define EEPROM_SIZE   64    // размер EEPROM

#include <WebSocketsClient.h>
#include <EEPROM.h>

#include <OLED_I2C.h>

#include "SSID_client.h"      // При необходимости изменить название и паролт WiFi точки доступа
#include "client.h"
#include "128x64.h"

// Подключение Дисплея
OLED myOLED(SDA_PIN, SCL_PIN);  // Remember to add the RESET pin if your display requires it...
extern uint8_t SmallFont[];
extern uint8_t BigNumbers[];

void printWifiState() {
  if (Connected == true) {
    myOLED.setFont(SmallFont);
    myOLED.print("Wifi: ON ", 0, 57);
  } else {
    myOLED.setFont(SmallFont);
    myOLED.print("Wifi: OFF", 0, 57);
  }
}

void printWsState() {
  if (ConnectedWS == true) {
    myOLED.setFont(SmallFont);
    myOLED.print("/ WS: ON ", 64, 57);
  } else {
    myOLED.setFont(SmallFont);
    myOLED.print("/ WS: OFF", 64, 57);
  }
}

void printSSID() {
  myOLED.setFont(SmallFont);
  myOLED.print("                      ", 0, 49);
  myOLED.print("Wifi: " + String(ssid[wifi_id]) + "     ", 0, 49);
  myOLED.update();
}

void drawDigit(int ddX, int digit) {
  for (int y = 0; y < SizeY; y++) {
    for (int x = 0; x < SizeX; x++) {
      if (digits[Font_ID][digit][y][x] == 1) {
        myOLED.setPixel(x + ddX, y);
      } else {
        myOLED.clrPixel(x + ddX, y);
      }
    }
  }
}

void PrintTime() {

  if (Font_ID == Font_Count) {
    
    myOLED.setFont(BigNumbers);
    myOLED.print(String(CurrentMinutes) + "-" + String(CurrenttSeconds) + String(Currentseconds) + "." + String(CurrentmSeconds) + String(CurrentmiSeconds) + String(CurrentmilSeconds), 8, 8);

  } else {
    int dX = 1;
    // Minutes
    drawDigit(dX + 0, CurrentMinutes);
    // Seconds
    drawDigit(dX + (1 * SizeX) + 5, CurrenttSeconds);
    drawDigit(dX + (2 * SizeX) + 6, Currentseconds);
    // milliseconds
    drawDigit(dX + (3 * SizeX) + 11, CurrentmSeconds);
    drawDigit(dX + (4 * SizeX) + 12, CurrentmiSeconds);
    drawDigit(dX + (5 * SizeX) + 13, CurrentmilSeconds);
    // Двоеточие
    myOLED.drawRect(dX + (1 * SizeX) + 1, int(SizeY / 3), dX + (1 * SizeX) + 3, int(SizeY / 3)+2);
    myOLED.setPixel(dX + (1 * SizeX) + 2, int(SizeY / 3) + 1);

    myOLED.drawRect(dX + (1 * SizeX) + 1, int(SizeY - SizeY / 3), dX + (1 * SizeX) + 3, int(SizeY - SizeY / 3) + 2);
    myOLED.setPixel(dX + (1 * SizeX) + 2, int(SizeY - SizeY / 3) + 1);
    // Точка
    myOLED.drawRect(dX + (3 * SizeX) + 7, SizeY - 3, dX + (3 * SizeX) + 9, SizeY - 1);
    myOLED.setPixel(dX + (3 * SizeX) + 8, SizeY - 2);
  }
}

void TimePrintXY(uint32_t time, byte x, byte y, String name) {

  uint8_t minutes = (int)(time / 60000) % 10;
  uint8_t tSeconds = (int)(time % 60000 / 10000);
  uint8_t seconds = (int)(time % 60000 % 10000 / 1000);
  uint8_t mSeconds = (int)(time % 60000 % 1000 / 100);
  uint8_t miSeconds = (int)(time % 60000 % 100 / 10);
  uint8_t milSeconds = (int)(time % 60000 % 10);
  myOLED.setFont(SmallFont);
  myOLED.print("                      ", x, y);
  myOLED.print(name + String(minutes) + ":" + String(tSeconds) + String(seconds) + "." + String(mSeconds) + String(miSeconds) + String(milSeconds) + "           ", x, y);
}

void FontChangeLoop() {
  Font_State = digitalRead(FONT_PIN);
  if (Font_State == LOW && Font_LastState == HIGH) {
    if (Font_ID >= 0 && Font_ID < Font_Count) {
      Font_ID++;
    } else {
      Font_ID = 0;
    }
    myOLED.clrScr();
    EEPROM.write(3, Font_ID);
    EEPROM.commit();
  }
  Font_LastState = Font_State;
}

void LastTimeIDChangeLoop() {
  LastTimeID_State = digitalRead(LAST_TIME_PIN);
  if (LastTimeID_State == LOW && LastTimeID_LastState == HIGH) {
    if (LastTimeID >= 0 && LastTimeID < LastTimeCount - 1) {
      LastTimeID++;
    } else {
      LastTimeID = 0;
    }
    LastTimeID_State_ts = millis();
  }
  LastTimeID_LastState = LastTimeID_State;
}

void PrintCopyright(void) {
  myOLED.clrScr();
  myOLED.setFont(SmallFont);
  myOLED.print("Made by VeZhD", 0, 48);
  myOLED.print("Code from Alekssaff", 0, 56);
  myOLED.update();
  delay(1500);
  ssid_state_ts = millis();
}

void setup() {
  pinMode(SSID_PIN, INPUT_PULLUP);
  pinMode(FONT_PIN, INPUT_PULLUP);       // пин кнопки переключения шрифта
  pinMode(LAST_TIME_PIN, INPUT_PULLUP);  // пин кнопки переключения предыдущего времени(1-10)

#if defined(ESP32)
  // For ESP32/ESP32s2
  if (!EEPROM.begin(EEPROM_SIZE)) {
    delay(1000000);
  } 
#elif defined(ESP8266)
  // For ESP8266
  EEPROM.begin(EEPROM_SIZE);
#endif

  if ((EEPROM.read(3) >= 0) && (EEPROM.read(3) <= Font_Count)) {
    Font_ID = EEPROM.read(3);
  }
  if ((EEPROM.read(0) >= 0) && (EEPROM.read(0) < (sizeof(ssid) / sizeof(char *)))) {
    wifi_id = EEPROM.read(0);
  }

  if (!myOLED.begin(SSD1306_128X64))
    while (1)
      ;  // In case the library failed to allocate enough RAM for the display buffer...

  //myOLED.rotateDisplay(true); // переворот на 180 градусов
  //myOLED.setBrightness(255);
  WiFi.onEvent(WiFiEvent);

  // event handler
  webSocket.onEvent(webSocketEvent);
  PrintCopyright();

  myOLED.clrScr();
  connectToHost();
}

void loop() {
  TimerLoop();
  ssidChangeLoop();
  FontChangeLoop();
  LastTimeIDChangeLoop();
  webSocket.loop();
  printWifiState();
  printWsState();
  PrintTime();

 if (LastTimeID_State_ts < millis() - 10500) {
    LastTimeID = 0;
  }

  TimePrintXY(LastTime[LastTimeID], 0, 40, "LastTime " + String(LastTimeID) + ": ");
  if (ssid_state_ts + 10000 < millis()) {
    TimePrintXY(TopTime, 0, 49, "  Top Time: ");
  } else {
    printSSID();
  }
  myOLED.update();
  delay(1);
}
