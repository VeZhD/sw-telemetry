//https://espressif.github.io/arduino-esp32/package_esp32_index.json
/*
Add to ESP32-HUB75-MatrixPanel-I2S-DMA.h :

#define PIXEL_COLOUR_DEPTH_BITS 2
#define NO_GFX
#define USE_GFX_ROOT
#define NO_FAST_FUNCTIONS
*/
#define VEZHD

#define DISPLAY_LASTTIME 2 // кол-во отображаемых значений последнего времени, обычно 1
#define SW_Basic_OTA_HOSTNAME SWC_RGB12864  // HostName для ESP
//#define SW_Basic_OTA_PASSWORD passwordSWC_RGB12864  // пароль для OTA обновления, по умолчанию "passwordSW_client", без ковычек

#define SSID_PIN 34        // пин кнопки переключения wifi сети
#define FONT_PIN 36        // пин кнопки переключения шрифта
#define LAST_TIME_PIN 38   // пин кнопки переключения предыдущего времени(1-10)
#define BRIGHTNESS_PIN 40  // пин кнопки яркости

#define EEPROM_SIZE 64

#include <WiFi.h>
#include <WebSocketsClient.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#include "SSID_client.h"      // При необходимости изменить название и паролт WiFi точки доступа
#include "client.h"
#include "128x64.h"

// Подключение Матрицы
#define R1_PIN 1
#define G1_PIN 3
#define B1_PIN 2
#define R2_PIN 4
#define G2_PIN 5
#define B2_PIN 6
#define E_PIN 7
#define A_PIN 8
#define B_PIN 9
#define C_PIN 10
#define D_PIN 11
#define CLK_PIN 13
#define LAT_PIN 12
#define OE_PIN 14

// настройки размера матрицы
#define MATRIX_WIDTH 128  // Number of pixels wide of each INDIVIDUAL panel module.
#define MATRIX_HEIGHT 64  // Number of pixels tall of each INDIVIDUAL panel module.
#define CHAIN_LENGTH 1    // Total number of panels chained one to another
HUB75_I2S_CFG::i2s_pins _pins = { R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN };

HUB75_I2S_CFG mxconfig(
  MATRIX_WIDTH,   // Module width
  MATRIX_HEIGHT,  // Module height
  CHAIN_LENGTH,   // chain length
  _pins, HUB75_I2S_CFG::SHIFTREG, true);

MatrixPanel_I2S_DMA *dma_display = new MatrixPanel_I2S_DMA(mxconfig);
// end подключение матрицы

uint16_t TIME_COLOR = dma_display->color444(0, 255, 0);
uint16_t DOT_COLOR = dma_display->color444(0, 255, 0);
const uint16_t BLACK_COLOR = dma_display->color444(0, 0, 0);

byte Brightness = 155;
bool Brightness_State = HIGH;
bool Brightness_LastState = HIGH;
uint32_t Brightness_State_ts = 0;


void updateDisplay() {
  dma_display->flipDMABuffer();  // Show the back buffer, set currently output buffer to the back (i.e. no longer being sent to LED panels)
  dma_display->clearScreen();    // Now clear the back-buffer
}

void printWifiState() {
  if (Connected == true) {
    dma_display->fillRect(int(MATRIX_WIDTH - 3), int(MATRIX_HEIGHT - 7), 3, 3, dma_display->color444(0, 255, 0));
  } else {
    dma_display->fillRect(int(MATRIX_WIDTH - 3), int(MATRIX_HEIGHT - 7), 3, 3, dma_display->color444(255, 0, 0));
  }
}

void printWsState() {
  if (ConnectedWS == true) {
    dma_display->fillRect(int(MATRIX_WIDTH - 3), int(MATRIX_HEIGHT - 3), 3, 3, dma_display->color444(0, 255, 0));
  } else {
    dma_display->fillRect(int(MATRIX_WIDTH - 3), int(MATRIX_HEIGHT - 3), 3, 3, dma_display->color444(255, 0, 0));
  }
}

void PrintSSID() {
  dma_display->setCursor(1, int(MATRIX_HEIGHT - 15));
  dma_display->setTextSize(1);  // size 1 == 8 pixels high
  dma_display->setTextColor(dma_display->color444(222, 222, 222));
  dma_display->print(String(ssid[wifi_id]));
}

void printBrightness() {
  dma_display->setTextColor(dma_display->color444(100, 100, 100));
  dma_display->setCursor(int(MATRIX_WIDTH - 6 * 3), int(MATRIX_HEIGHT - 15));  // start at top left, with 8 pixel of spacing
  dma_display->setTextSize(1);
  dma_display->print(Brightness);
}

void drawDigit(int ddX, int digit) {
  for (int y = 0; y < SizeY; y++) {
    for (int x = 0; x < SizeX; x++) {
      if (digits[Font_ID][digit][y][x] == 1) {
        dma_display->drawPixel(x + ddX, y, TIME_COLOR);
        /*
        }
      else {
        dma_display->drawPixel(x + ddX, y, BLACK_COLOR);
        */
      }
    }
  }
}

void PrintTime() {

  if (StartStopState == 0) {
    TIME_COLOR = dma_display->color444(255, 0, 0);
    DOT_COLOR = dma_display->color444(0, 255, 0);
  } else {
    TIME_COLOR = dma_display->color444(0, 255, 0);
    DOT_COLOR = dma_display->color444(255, 0, 0);
  }

  if (Font_ID == Font_Count) {
    const uint8_t font_size = 4;
    dma_display->setTextWrap(false);  // Don't wrap at end of line - will do ourselves
    dma_display->setCursor(0, 1);     // start at top left, with 8 pixel of spacing
    dma_display->setTextColor(TIME_COLOR);
    dma_display->setTextSize(font_size);  // size 1 == 8 pixels high
    dma_display->print(CurrentMinutes);
    dma_display->fillRect(5 * font_size, 2 * font_size + 1, 3, 3, DOT_COLOR);  // двоеточие
    dma_display->fillRect(5 * font_size, 4 * font_size + 2, 3, 3, DOT_COLOR);  // двоеточие
    dma_display->setCursor(5 * font_size + 4, 1);
    dma_display->print(CurrenttSeconds);
    dma_display->setCursor(10 * font_size + 5, 1);
    dma_display->print(Currentseconds);
    dma_display->fillRect(15 * font_size + 4, 6 * font_size + 1, font_size, font_size, DOT_COLOR);  // точка перед милисикундами
    dma_display->setCursor(15 * font_size + 7, 1);
    dma_display->print(CurrentmSeconds);
    dma_display->setCursor(20 * font_size + 8, 1);
    dma_display->print(CurrentmiSeconds);
    dma_display->setCursor(25 * font_size + 9, 1);
    dma_display->print(CurrentmilSeconds);
  } else {
    int dX = 0;
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
    dma_display->fillRect(dX + (1 * SizeX) + 1, int(SizeY / 3), 3, 3, DOT_COLOR);
    dma_display->fillRect(dX + (1 * SizeX) + 1, int(SizeY - SizeY / 3), 3, 3, DOT_COLOR);
    // Точка
    dma_display->fillRect(dX + (3 * SizeX) + 7, SizeY - 3, 3, 3, DOT_COLOR);
  }
}

void PrintTimeSeconds() {
  const uint8_t font_size = 8;
  const uint8_t dy = 4;
  const uint8_t dot = (font_size / 2 + 1);

  dma_display->setTextWrap(false);  // Don't wrap at end of line - will do ourselves
  dma_display->setCursor(0, dy);
  dma_display->setTextColor(TIME_COLOR);
  dma_display->setTextSize(font_size);
  dma_display->print(CurrentMinutes);
  dma_display->fillRect(5 * font_size + 1, 2 * font_size + 1, dot, dot, DOT_COLOR);  // двоеточие
  dma_display->fillRect(5 * font_size + 1, 4 * font_size + 2, dot, dot, DOT_COLOR);  // двоеточие
  dma_display->setCursor(5 * font_size + 7, dy);
  dma_display->print(CurrenttSeconds);
  dma_display->setCursor(10 * font_size + 8, dy);
  dma_display->print(Currentseconds);
}

void PrintTimeMilSeconds() {
  const uint8_t font_size = 8;
  const uint8_t dy = 4;
  const uint8_t dot = (font_size / 2 + 1);

  dma_display->setTextWrap(false);  // Don't wrap at end of line - will do ourselves
  dma_display->setCursor(0, dy);
  dma_display->setTextColor(TIME_COLOR);
  dma_display->setTextSize(font_size);
  dma_display->fillRect(0, 6 * font_size + dot + dy / 2, dot, dot, DOT_COLOR);  // точка перед милисикундами
  dma_display->setCursor(dot + 1, dy);
  dma_display->print(CurrentmSeconds);
  dma_display->setCursor(5 * font_size + dot + 2, dy);
  dma_display->print(CurrentmiSeconds);
  dma_display->setCursor(10 * font_size + dot + 3, dy);
  dma_display->print(CurrentmilSeconds);
}

void TimePrintXY(uint32_t time, byte x, byte y, String name) {

  uint8_t minutes = (int)(time / 60000) % 10;
  uint8_t tSeconds = (int)(time % 60000 / 10000);
  uint8_t seconds = (int)(time % 60000 % 10000 / 1000);
  uint8_t mSeconds = (int)(time % 60000 % 1000 / 100);
  uint8_t miSeconds = (int)(time % 60000 % 100 / 10);
  uint8_t milSeconds = (int)(time % 60000 % 10);

  dma_display->setCursor(x, y);  // 1 : 58
  dma_display->setTextWrap(false);
  dma_display->setTextColor(TIME_COLOR);
  dma_display->setTextSize(1);
  dma_display->print(name);
  dma_display->setTextColor(dma_display->color444(255, 0, 0));
  dma_display->drawPixel(x + name.length() * 6, y + 2, dma_display->color444(0, 0, 255));  // двоеточие
  dma_display->drawPixel(x + name.length() * 6, y + 4, dma_display->color444(0, 0, 255));  // двоеточие
  dma_display->setCursor(x + name.length() * 6 + 2, y);
  dma_display->print(minutes);
  dma_display->drawPixel(x + name.length() * 6 + 8, y + 2, dma_display->color444(0, 0, 255));  // двоеточие
  dma_display->drawPixel(x + name.length() * 6 + 8, y + 4, dma_display->color444(0, 0, 255));  // двоеточие
  dma_display->setCursor(x + name.length() * 6 + 10, y);
  dma_display->print(tSeconds);
  dma_display->print(seconds);
  dma_display->drawPixel(x + name.length() * 6 + 22, y + 6, dma_display->color444(0, 0, 255));  // точка перед милисикундами
  dma_display->setCursor(x + name.length() * 6 + 24, y);
  dma_display->print(mSeconds);
  dma_display->print(miSeconds);
  dma_display->print(milSeconds);
}

void FontChangeLoop() {
  Font_State = digitalRead(FONT_PIN);
  if (Font_State == LOW && Font_LastState == HIGH) {
    if (Font_ID >= 0 && Font_ID < Font_Count + 2) {
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
      Brightness = 125;
    }

    dma_display->setBrightness(Brightness);
    EEPROM.write(5, Brightness);
    EEPROM.commit();
    Brightness_State_ts = millis();
  }
  Brightness_LastState = Brightness_State;
}

void PrintCopyright(void) {
  dma_display->setTextColor(TIME_COLOR);
  dma_display->setCursor(8, 8);  // int(MATRIX_HEIGHT - 16));
  dma_display->print("Made by VeZhD");
  dma_display->setCursor(5, 16);  // int(MATRIX_HEIGHT - 8));
  dma_display->print("Code from Alekssaff");
  updateDisplay();
  delay(1500);
  dma_display->clearScreen();
}

void setup() {
  pinMode(SSID_PIN, INPUT_PULLUP);       // пин кнопки переключения точки доступа
  pinMode(FONT_PIN, INPUT_PULLUP);       // пин кнопки переключения шрифта
  pinMode(LAST_TIME_PIN, INPUT_PULLUP);  // пин кнопки переключения предыдущего времени(1-10)
  pinMode(BRIGHTNESS_PIN, INPUT_PULLUP); // пин кнопки яркости

  if (!EEPROM.begin(EEPROM_SIZE)) {
    delay(1000000);
  }
  if ((EEPROM.read(3) >= 0) && (EEPROM.read(3) <= Font_Count)) {
    Font_ID = EEPROM.read(3);
  }
  if ((EEPROM.read(0) >= 0) && (EEPROM.read(0) < (sizeof(ssid) / sizeof(char *)))) {
    wifi_id = EEPROM.read(0);
  }
  if ((EEPROM.read(5) >= 0) && (EEPROM.read(5) <= 255)) {
    Brightness = EEPROM.read(5);
  }

  WiFi.onEvent(WiFiEvent);

  dma_display->begin();
  dma_display->clearScreen();
  dma_display->setBrightness(Brightness);  //0-255
  //dma_display->setLatBlanking(0);

  // event handler
  webSocket.onEvent(webSocketEvent);
  updateDisplay();
  PrintCopyright();
  updateDisplay();
  connectToHost();
  SW_Basic_OTA();
}

void loop() {
  ArduinoOTA.handle();

  TimerLoop();
  ssidChangeLoop();
  FontChangeLoop();
  LastTimeIDChangeLoop();
  BrightnessChangeLoop();
  webSocket.loop();

  printWifiState();
  printWsState();

  switch (Font_ID) {
    case (Font_Count + 1):
      PrintTimeSeconds();
      break;
    case (Font_Count + 2):
      PrintTimeMilSeconds();
      break;
    default:
      PrintTime();
      TimePrintXY(LastTime[LastTimeID], 0, 57, "LT" + String(LastTimeID));
      TimePrintXY(LastTime[LastTimeID + 1], 64, 57, "LT" + String(LastTimeID + 1));

      if (ssid_state_ts + 5500 > millis() ) {
        PrintSSID();
      } else {
        TimePrintXY(TopTime, 11, 48, "Top Time");
      }
      if (Brightness_State_ts + 5500 > millis() ) {
        printBrightness();
      }
  }

  updateDisplay();
  //delay(1);
}
