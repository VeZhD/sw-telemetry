/*********
display OLED i2c 128*64 witch WEMOS(LOLIN) S2 mini
SCL -> D1(GPIO5) For ESP8266(Wemos D1 mini) / 35 For ESP32s2(Wemos(Lolin) S2 mini)
SDA -> D2(GPIO4) For ESP8266(Wemos D1 mini) / 33 For ESP32s2(Wemos(Lolin) S2 mini)
VCC -> 5V
GND -> GND
*********/
#define DEFAULT_00
#define SW_Basic_OTA_HOSTNAME "SWC_WS2812b"  // HostName для ESP, по умолчанию "SW_client", без ковычек
//#define SW_Basic_OTA_PASSWORD "passwordSWC_WS2812b"  // пароль для OTA обновления, по умолчанию "passwordSW_client", без ковычек

#include <FastLED.h>

#if defined(ESP32)
  #pragma message "ESP32 stuff happening!"

  #define SSID_PIN        17  // пин кнопки переключения wifi сети
  #define FONT_PIN        21  // пин кнопки переключения шрифта
  #define LAST_TIME_PIN   34  // пин кнопки переключения предыдущего времени(1-10)
  #define BRIGHTNESS_PIN  36  // пин кнопки яркости

  #define DATA_PIN        17  // пин подключния матрицы

  #include <WiFi.h>

#elif defined(ESP8266)
  #pragma message "ESP8266 stuff happening!"

  #define SSID_PIN        16  // пин кнопки переключения wifi сети
  #define FONT_PIN        14  // пин кнопки переключения шрифта
  #define LAST_TIME_PIN   12  // пин кнопки переключения предыдущего времени(1-10)
  #define BRIGHTNESS_PIN  13  // пин кнопки яркости

  #define DATA_PIN         12  // пин подключения матрице

  #include <ESP8266WiFi.h>

#else
#error "This ain't a ESP8266 or ESP32, dumbo!"
#endif

#define EEPROM_SIZE   64    // размер EEPROM

#include <WebSocketsClient.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>

#include "SSID_client.h"      // При необходимости изменить название и паролт WiFi точки доступа
#include "client.h"
#include "32x8.h"

byte Brightness = 20;
bool Brightness_State = HIGH;
bool Brightness_LastState = HIGH;
uint32_t Brightness_State_ts = 0;

uint32_t DotColor = CRGB::Black;
uint32_t NumbersColor = CRGB::Black;

#define NUM_LEDS 256

// Описание матрицы, возможно есть лишнее
#define CONNECTION_ANGLE 1
#define STRIP_DIRECTION 3
#define MATRIX_TYPE 0
#define COLOR_ORDER GRB
#define WIDTH 32
#define HEIGHT 8
#define SEGMENTS 1

CRGB leds[NUM_LEDS];

#if (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define THIS_X (HEIGHT - y - 1)
#define THIS_Y x

#else
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y y
#pragma message "Wrong matrix parameters! Set to default"

#endif

// функция отрисовки точки по координатам X Y
void drawPixelXY(int x, int y, uint32_t color) {
  leds[getPixelNumber(x, y)] = color;
}

// получить номер пикселя в ленте по координатам
uint16_t getPixelNumber(int x, int y) {
  if ((THIS_Y % 2 == 0) || MATRIX_TYPE) {  // если чётная строка
    return (THIS_Y * _WIDTH + THIS_X);
  } else {  // если нечётная строка
    return (THIS_Y * _WIDTH + _WIDTH - THIS_X - 1);
  }
}

void printWifiState() {
  if (Connected == true) {
    NumbersColor = CRGB::Green;
    //myOLED.setFont(SmallFont);
    //myOLED.print("Wifi: ON ", 0, 57);
  } else {
    NumbersColor = CRGB::White;
    //myOLED.setFont(SmallFont);
    //myOLED.print("Wifi: OFF", 0, 57);
  }
}

void printWsState() {
  if (ConnectedWS == true) {
    DotColor = CRGB::Red;
    //myOLED.setFont(SmallFont);
    //myOLED.print("/ WS: ON ", 64, 57);
  } else {
    DotColor = CRGB::Black;
    //myOLED.setFont(SmallFont);
    //myOLED.print("/ WS: OFF", 64, 57);
  }
}

void printSSID() {
  //myOLED.setFont(SmallFont);
  //myOLED.print("                      ", 0, 49);
  //myOLED.print("Wifi: " + String(ssid[wifi_id]) + "     ", 0, 49);
  //myOLED.update();
}

void drawDigit(int dX, int digit, uint32_t color) {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 4; x++) {
      if (digits[digit][y][x] == 1) {
        drawPixelXY(x + dX, abs(y - 7), color);
      }
      /*
      else {
        drawPixelXY(x + dX, abs(y - 7), CRGB::Black);
      }
      */
    }
  }
}
void PrintTime() {

  int dX = 0;
  // minutes
  drawDigit(dX, CurrentMinutes, NumbersColor);
  // dots
  dX = 4;
  drawPixelXY(1 + dX, 3, DotColor);
  drawPixelXY(1 + dX, 5, DotColor);

  // tSeconds
  dX = 7;
  drawDigit(dX, CurrenttSeconds, NumbersColor);
  // seconds
  dX = 12;
  drawDigit(dX, Currentseconds, NumbersColor);
  // dot
  dX = 16;
  drawPixelXY(1 + dX, 0, DotColor);
  //drawDot(dX, NumbersColor);
  dX = 18;
  drawDigit(dX, CurrentmSeconds, NumbersColor);
  dX = 23;
  drawDigit(dX, CurrentmiSeconds, NumbersColor);
  dX = 28;
  drawDigit(dX, CurrentmilSeconds, NumbersColor);
}

void TimePrintXY(uint32_t time, byte x, byte y, String name) {

  uint8_t minutes = (int)(time / 60000) % 10;
  uint8_t tSeconds = (int)(time % 60000 / 10000);
  uint8_t seconds = (int)(time % 60000 % 10000 / 1000);
  uint8_t mSeconds = (int)(time % 60000 % 1000 / 100);
  uint8_t miSeconds = (int)(time % 60000 % 100 / 10);
  uint8_t milSeconds = (int)(time % 60000 % 10);
  //myOLED.setFont(SmallFont);
  //myOLED.print("                      ", x, y);
  //myOLED.print(name + String(minutes) + ":" + String(tSeconds) + String(seconds) + "." + String(mSeconds) + String(miSeconds) + String(milSeconds) + "           ", x, y);
}

void FontChangeLoop() {

  Font_State = digitalRead(FONT_PIN);
  if (Font_State == LOW && Font_LastState == HIGH) {
    if (Font_ID >= 0 && Font_ID < Font_Count) {
      Font_ID++;
    } else {
      Font_ID = 0;
    }
    //myOLED.clrScr();
    EEPROM.write(3, Font_ID);
    EEPROM.commit();
  }
  Font_LastState = Font_State;
}

void PrintCopyright(void) {
  //myOLED.clrScr();
  //myOLED.setFont(SmallFont);
  //myOLED.print("Made by VeZhD", 0, 48);
  //myOLED.print("Code from Alekssaff", 0, 56);
  //myOLED.update();
  delay(1500);
  ssid_state_ts = millis();
}

void BrightnessChangeLoop() {
  Brightness_State = digitalRead(BRIGHTNESS_PIN);
  if (Brightness_State == LOW && Brightness_LastState == HIGH) {
    if (Brightness > 0 && Brightness < 255) {
      Brightness += 5;
    } else {
      Brightness = 155;
    }

    FastLED.setBrightness(Brightness);
    EEPROM.write(5, Brightness);
    EEPROM.commit();
    Brightness_State_ts = millis();
  }
  Brightness_LastState = Brightness_State;
}

void setup() {
  pinMode(SSID_PIN, INPUT_PULLUP);
  pinMode(FONT_PIN, INPUT_PULLUP);       // пин кнопки переключения шрифта
  pinMode(LAST_TIME_PIN, INPUT_PULLUP);  // пин кнопки переключения предыдущего времени(1-10)

  FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, NUM_LEDS);
  //FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(Brightness);

  
  
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

  WiFi.onEvent(WiFiEvent);

  // event handler
  webSocket.onEvent(webSocketEvent);
  //PrintCopyright();

  connectToHost();
  SW_Basic_OTA();

}

void loop() {
  webSocket.loop();

  ArduinoOTA.handle();
  FastLED.clear();
  TimerLoop();
  //ssidChangeLoop();
  //FontChangeLoop();
  //LastTimeIDChangeLoop();
  //BrightnessChangeLoop();
  printWifiState();
  printWsState();
  PrintTime();

  //TimePrintXY(LastTime[LastTimeID], 0, 40, "LastTime " + String(LastTimeID) + ": ");
  
  //if (ssid_state_ts + 5000 > millis()) {
  //  printSSID();
  //} else {
  //  TimePrintXY(TopTime, 0, 49, "Top Time: ");
  //}
  FastLED.show();
  //FastLED.delay(10);
  //delay(1);
}
