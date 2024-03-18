/*********
display OLED i2c 128*64 witch WEMOS(LOLIN) S2 mini
SCL -> D1(GPIO5) For ESP8266(Wemos D1 mini) / 35 For ESP32s2(Wemos(Lolin) S2 mini)
SDA -> D2(GPIO4) For ESP8266(Wemos D1 mini) / 33 For ESP32s2(Wemos(Lolin) S2 mini)
VCC -> 5V
GND -> GND
*********/
#define SW_Basic_OTA_HOSTNAME "SWC_e-INK"  // HostName для ESP, по умолчанию "SW_client", без ковычек
//#define SW_Basic_OTA_PASSWORD "passwordSWC_OLED"  // пароль для OTA обновления, по умолчанию "passwordSW_client", без ковычек

#define BUSY_PIN  39
#define RST_PIN   37  // Reset
#define DC_PIN    35  // D/C
#define CS_PIN    33
#define CLK_PIN   18  // SCLK
#define DIN_PIN   16  // SDI

#define SSID_PIN      34   // пин кнопки переключения wifi сети
#define FONT_PIN      36   // пин кнопки переключения шрифта
#define LAST_TIME_PIN 38   // пин кнопки переключения предыдущего времени(1-10)

#define EEPROM_SIZE   64    // размер EEPROM

#include <WiFi.h>
#include <WebSocketsClient.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <U8g2_for_Adafruit_GFX.h>

// Выбор дисплея - размер, версия, кол-во цветов
// select the display class and display driver class in the following file (new style):
#include "GxEPD2_display_selection_new_style.h"

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

#include "SSID_client.h"      // При необходимости изменить название и паролт WiFi точки доступа
#include "client.h"

uint32_t PreviousPrintTime = millis();
uint PrintDelay = 555;
uint16_t bg = GxEPD_WHITE;
uint16_t fg = GxEPD_BLACK;

void printWifiState() {
  if (Connected == true) {
    u8g2Fonts.setFont(u8g2_font_9x15_tf);
    u8g2Fonts.setCursor(0, 10);
    u8g2Fonts.print("Wifi: ON ");
  } else {
    u8g2Fonts.setFont(u8g2_font_9x15_tf);
    u8g2Fonts.setCursor(0, 10);
    u8g2Fonts.print("Wifi: OFF");
  }
}

void printWsState() {
  if (ConnectedWS == true) {
    u8g2Fonts.setFont(u8g2_font_9x15_tf);
    u8g2Fonts.setCursor(0, 99);
    u8g2Fonts.print("/ WS: ON ");
  } else {
    u8g2Fonts.setFont(u8g2_font_9x15_tf);
    u8g2Fonts.setCursor(0, 99);
    u8g2Fonts.print("/ WS: OFF");
  }
}

void printSSID() {
  u8g2Fonts.setFont(u8g2_font_9x15_tf);
  u8g2Fonts.setCursor(35, 10);
  u8g2Fonts.print("Wifi: " + String(ssid[wifi_id]));
}

void PrintTime() {
  u8g2Fonts.setFont(u8g2_font_logisoso62_tn);  
  u8g2Fonts.setCursor(55, 0);
  u8g2Fonts.print(String(CurrentMinutes));
  u8g2Fonts.setCursor(55, 29);
  u8g2Fonts.print(":");
  u8g2Fonts.setCursor(55, 42);
  u8g2Fonts.print(String(CurrenttSeconds) + String(Currentseconds));
  u8g2Fonts.setCursor(55, 113);
  u8g2Fonts.print(".");
  u8g2Fonts.setCursor(55, 127);
  u8g2Fonts.print(String(CurrentmSeconds) + String(CurrentmiSeconds) + String(CurrentmilSeconds));

}

void TimePrintXY(uint32_t time, byte x, byte y, String name) {

  uint8_t minutes = (int)(time / 60000) % 10;
  uint8_t tSeconds = (int)(time % 60000 / 10000);
  uint8_t seconds = (int)(time % 60000 % 10000 / 1000);
  uint8_t mSeconds = (int)(time % 60000 % 1000 / 100);
  uint8_t miSeconds = (int)(time % 60000 % 100 / 10);
  uint8_t milSeconds = (int)(time % 60000 % 10);

  u8g2Fonts.setFont(u8g2_font_9x15_tf);
  u8g2Fonts.setCursor(x, y);
  u8g2Fonts.print(name + String(minutes) + ":" + String(tSeconds) + String(seconds) + "." + String(mSeconds) + String(miSeconds) + String(milSeconds));
}

void setup() {
  pinMode(SSID_PIN, INPUT_PULLUP);
  pinMode(FONT_PIN, INPUT_PULLUP);       // пин кнопки переключения шрифта
  pinMode(LAST_TIME_PIN, INPUT_PULLUP);  // пин кнопки переключения предыдущего времени(1-10)
  
  SPI.begin(CLK_PIN, -1, DIN_PIN, CS_PIN);
  //display.init(115200); // default 10ms reset pulse, e.g. for bare panels with DESPI-C02
  display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  u8g2Fonts.begin(display); // connect u8g2 procedures to Adafruit GFX
  u8g2Fonts.setFontMode(1);                 // use u8g2 transparent mode (this is default)
  u8g2Fonts.setFontDirection(1);            // left to right (this is default)
  u8g2Fonts.setForegroundColor(fg);         // apply Adafruit GFX color
  u8g2Fonts.setBackgroundColor(bg);         // apply Adafruit GFX color
  display.fillScreen(bg);

  if (!EEPROM.begin(EEPROM_SIZE)) {
    delay(1000000);
  } 

  if ((EEPROM.read(0) >= 0) && (EEPROM.read(0) < (sizeof(ssid) / sizeof(char *)))) {
    wifi_id = EEPROM.read(0);
  }

  WiFi.onEvent(WiFiEvent);

  // event handler
  webSocket.onEvent(webSocketEvent);

  connectToHost();
  SW_Basic_OTA();
}

void loop() {
  ArduinoOTA.handle();

  TimerLoop();
  ssidChangeLoop();
  //FontChangeLoop();
  LastTimeIDChangeLoop();
  webSocket.loop();
  if (millis() - PreviousPrintTime >= PrintDelay) {
    PreviousPrintTime = millis();
    
    display.fillScreen(bg);
    PrintTime();
    printWifiState();
    printWsState();
  
    TimePrintXY(LastTime[LastTimeID], 17, 10, "LastTime " + String(LastTimeID) + ": ");

    if (ssid_state_ts + 7500 > millis()) {
      printSSID();
    } else {
      TimePrintXY(TopTime, 35, 28, "Top Time: ");
    }
  
    display.display(true); // full update
  }
  //delay(5);
}
