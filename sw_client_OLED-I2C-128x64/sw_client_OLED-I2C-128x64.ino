/*********
display OLED i2c 128*64 witch WEMOS(LOLIN) S2 mini
SCL -> 35 For ESP32s2(Wemos(Lolin) S2 mini)
SDA -> 33 For ESP32s2(Wemos(Lolin) S2 mini)
VCC -> 5V
GND -> GND
*********/

#if defined(ESP32)
  #pragma message "ESP32 stuff happening!"
#else
#error "This ain't a  ESP32, dumbo!"
#endif

#define OLED_I2C

#define SSID_PIN      34   // пин кнопки переключения wifi сети
#define FONT_PIN      36   // пин кнопки переключения шрифта
#define LAST_TIME_PIN 38   // пин кнопки переключения предыдущего времени(1-10)

#define SDA_PIN 33 //change for yours SDA pin on ESP32\32s2\etc.
#define SCL_PIN 35 //change for yours SCL pin on ESP32\32s2\etc.

#include <WiFi.h>
#include <AsyncTCP.h>
#include <WebSocketsClient.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#include <OLED_I2C.h>

// Подключение Дисплея
OLED myOLED(SDA_PIN, SCL_PIN);  // Remember to add the RESET pin if your display requires it...
extern uint8_t SmallFont[];
extern uint8_t BigNumbers[];


uint8_t Font_ID = 0;
const uint8_t Font_Count = 3;

AsyncWebServer server(80);

#include "config.h"
#include "client.h"
#include "128x64_OLED.h"
#include "ota_update.h"

#include "html.h"
#include "favicon.h"
#include "routes.h"

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
  
  InitConfig();

  if ( digitalRead(SSID_PIN) == LOW ) {
    wifi_id = 0;
    wifiMode = "server";
    ssid_laststate = LOW;
  }

  // Init Display
  if (!myOLED.begin(SSD1306_128X64))
    while (1)
      ;  // In case the library failed to allocate enough RAM for the display buffer...

  //myOLED.rotateDisplay(true); // переворот на 180 градусов
  //myOLED.setBrightness(255);
  PrintCopyright();

  InitWifi();

  WiFi.onEvent(WiFiEvent);
  // event handler
  webSocket.onEvent(webSocketEvent);
  
  InitRoutes();

  OTAWeb_update_begin();

  server.begin();
  
  myOLED.clrScr();

}

void loop() {
  webSocket.loop();
  myOLED.clrScr();

  TimerLoop();

  get_IP();

  ssidChangeLoop();

  FontChangeLoop();
  
  LastTimeIDChangeLoop();

  if (printIP_ts > millis() ) {
    // myOLED.print(apIP, 0, 40);
    myOLED.print("IP: " + WiFi.localIP().toString(), 0, 40);
  } else if (LastTimeID <10 && wifiMode == "client") {
    TimePrintXY(LastTime[LastTimeID], 0, 40, "LastTime 0" + String(LastTimeID) + ": ");
  }
  else if ( wifiMode == "client") {
    TimePrintXY(LastTime[LastTimeID], 0, 40, "LastTime " + String(LastTimeID) + ": ");
  }
  
  if ( wifiMode == "server" ){
    myOLED.print("Chip: " + String(ESP.getChipModel()), 0, 0);
    // myOLED.print(ESP.getChipCores(), 0, 8);
    // myOLED.print(ESP.getChipRevision(), 0, 16);
    // myOLED.print(ESP.getEfuseMac(), 0, 24);
    myOLED.print("MAC:" + WiFi.macAddress(), 0, 8);
    myOLED.print(WiFi.getHostname(), 0, 16);
    myOLED.print(WiFi.softAPmacAddress(), 0, 24);


    myOLED.print("IP: " + WiFi.softAPIP().toString(), 0, 40);
    printSSID();
    printPass();

  } else if ( wifiMode == "client") { 
    printWifiState();
    printWsState();
    PrintTime();
  }

  if (ssid_state_ts > millis()) {
    printSSID();
  } else if ( wifiMode == "client") {
    TimePrintXY(TopTime, 0, 49, "Top Time: ");
  }

  myOLED.update();
  //delay(1);
  // delayMicroseconds(1);

}
