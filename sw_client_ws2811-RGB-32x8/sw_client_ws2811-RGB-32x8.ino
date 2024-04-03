/*********
display OLED i2c 128*64 witch WEMOS(LOLIN) S2 mini
SCL -> D1(GPIO5) For ESP8266(Wemos D1 mini) / 35 For ESP32s2(Wemos(Lolin) S2 mini)
SDA -> D2(GPIO4) For ESP8266(Wemos D1 mini) / 33 For ESP32s2(Wemos(Lolin) S2 mini)
VCC -> 5V
GND -> GND
*********/

#define SW_Basic_OTA_HOSTNAME "SWC_WS2812b"  // HostName для ESP, по умолчанию "SW_client", без ковычек
//#define SW_Basic_OTA_PASSWORD "passwordSWC_WS2812b"  // пароль для OTA обновления, по умолчанию "passwordSW_client", без ковычек

#if defined(ESP32)
  #pragma message "ESP32 stuff happening!"

  #define SSID_PIN        17  // пин кнопки переключения wifi сети
  #define FONT_PIN        21  // пин кнопки переключения шрифта
  #define LAST_TIME_PIN   34  // пин кнопки переключения предыдущего времени(1-10)
  #define BRIGHTNESS_PIN  36  // пин кнопки яркости

  #define DATA_PIN        16  // пин подключния матрицы

  #include <WiFi.h>

#elif defined(ESP8266)
  // с ESP8266 не было полноценных тестов, возможно что-то отвалится, возможно нужно переназначить пины
  #pragma message "ESP8266 stuff happening!"

  #define SSID_PIN        16  // пин кнопки переключения wifi сети
  #define FONT_PIN        14  // пин кнопки переключения шрифта
  #define LAST_TIME_PIN   12  // пин кнопки переключения предыдущего времени(1-10)
  #define BRIGHTNESS_PIN  13  // пин кнопки яркости

  #define DATA_PIN         4  // пин подключения матрице

  #include <ESP8266WiFi.h>

#else
#error "This ain't a ESP8266 or ESP32, dumbo!"
#endif

#define EEPROM_SIZE   64    // размер EEPROM

#include <WebSocketsClient.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>

#include <Adafruit_NeoMatrix.h>

#include "SSID_client.h"      // При необходимости изменить название и паролт WiFi точки доступа
#include "client.h"
#include "32x8.h"

byte Brightness = 5;
bool Brightness_State = HIGH;
bool Brightness_LastState = HIGH;
uint32_t Brightness_State_ts = 0;

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32, 8, DATA_PIN,
  NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB + NEO_KHZ800);

const uint16_t colors[] = {matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255)};

uint16_t DotColor = matrix.Color(255, 0, 0);
uint16_t NumbersColor = matrix.Color(0, 255, 0);

void printWifiState() {
  if (Connected == true) {
    NumbersColor = matrix.Color(0, 255, 0);
    //myOLED.setFont(SmallFont);
    //myOLED.print("Wifi: ON ", 0, 57);
  } else {
    NumbersColor = matrix.Color(0, 0, 255);
    //myOLED.setFont(SmallFont);
    //myOLED.print("Wifi: OFF", 0, 57);
  }
}

void printWsState() {
  if (ConnectedWS == true) {
    DotColor = matrix.Color(255, 0, 0);
    //myOLED.setFont(SmallFont);
    //myOLED.print("/ WS: ON ", 64, 57);
  } else {
    DotColor = matrix.Color(0, 0, 255);
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

void drawDigit(int dX, int digit, uint16_t color) {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 4; x++) {
      if (digits[digit][y][x] == 1) {
        matrix.drawPixel(x + dX, y, color);
      }
    }
  }
}

void PrintTime() {

  int dX = 0;
  // minutes
  drawDigit(dX, CurrentMinutes, NumbersColor);
  // dots
  dX = 4;
  matrix.drawPixel(1 + dX, 3, DotColor);
  matrix.drawPixel(1 + dX, 5, DotColor);

  // tSeconds
  dX = 7;
  drawDigit(dX, CurrenttSeconds, NumbersColor);
  // seconds
  dX = 12;
  drawDigit(dX, Currentseconds, NumbersColor);
  // dot
  dX = 16;
  matrix.drawPixel(1 + dX, 7, DotColor);
  //drawDot(dX, NumbersColor);
  dX = 18;
  drawDigit(dX, CurrentmSeconds, NumbersColor);
  dX = 23;
  drawDigit(dX, CurrentmiSeconds, NumbersColor);
  dX = 28;
  drawDigit(dX, CurrentmilSeconds, NumbersColor);
/*
  matrix.setCursor(0, 0);                     // Отступ сверху
  matrix.print(CurrentMinutes);
  matrix.setCursor(4, 0);                     // Отступ сверху
  matrix.print(":");
  matrix.setCursor(8, 0);                     // Отступ сверху
  matrix.print(String(CurrenttSeconds) + String(Currentseconds));
  matrix.setCursor(17, 0);                     // Отступ сверху
  matrix.print(".");
  matrix.setCursor(21, 0);                     // Отступ сверху
  matrix.print(String(CurrentmSeconds) + String(CurrentmiSeconds));
*/
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
    EEPROM.write(3, Font_ID);
    EEPROM.commit();
  }
  Font_LastState = Font_State;
}

void BrightnessChangeLoop() {
  Brightness_State = digitalRead(BRIGHTNESS_PIN);
  if (Brightness_State == LOW && Brightness_LastState == HIGH) {
    if (Brightness > 0 && Brightness < 255) {
      Brightness += 5;
    } else {
      Brightness = 55;
    }

    matrix.setBrightness(Brightness);
    EEPROM.write(5, Brightness);
    EEPROM.commit();
    Brightness_State_ts = millis();
  }
  Brightness_LastState = Brightness_State;
}

void PrintTicker(String text,uint16_t colors) {
  for (int x = matrix.width(); x > -73; x--){
    matrix.fillScreen(0);
    matrix.setCursor(x, 0);                     // Отступ сверху
    matrix.setTextColor(colors);
    matrix.print(text);
    matrix.show();                              // Функция показа текста
    delay(111);
  }
}

void setup() {
  pinMode(SSID_PIN, INPUT_PULLUP);
  pinMode(FONT_PIN, INPUT_PULLUP);       // пин кнопки переключения шрифта
  pinMode(LAST_TIME_PIN, INPUT_PULLUP);  // пин кнопки переключения предыдущего времени(1-10)

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(Brightness);
  matrix.setTextColor(colors[0]);
  
  
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

  connectToHost();
  SW_Basic_OTA();

  PrintTicker("Hello Gymkhanists", NumbersColor);

}

void loop() {
  webSocket.loop();

  ArduinoOTA.handle();
  matrix.fillScreen(0);
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
  matrix.show();                              // Функция показа текста
  //delay(1);
}
