#if defined(ESP32)
  #pragma message "ESP32 stuff happening!"
#else
#error "This ain't a  ESP32, dumbo!"
#endif

#define ws2811

#define SSID_PIN        10  // пин кнопки переключения wifi сети
#define FONT_PIN        11  // пин кнопки переключения шрифта
#define BRIGHTNESS_PIN  12  // пин кнопки яркости
#define BRIGHTNESS_less_PIN   13  // пин кнопки переключения предыдущего времени(1-10) // not used in this sketch

// #define LAST_TIME_PIN   14  // пин кнопки переключения предыдущего времени(1-10) - ХЗ ни где сейчас не используется 

#define DATA_PIN        16  // пин подключния матрицы

#include <WiFi.h>
#include <AsyncTCP.h>
#include <WebSocketsClient.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#include <Adafruit_NeoMatrix.h>

// Последовательность определения переменных и подключения следующих файлов важна, не менять местами:
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32, 8, DATA_PIN,
  NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB + NEO_KHZ800);

int shiftX = 0;

uint8_t Font_ID = 0;
const uint8_t Font_Count = 1;

AsyncWebServer server(80);

 #include "sw_config.h"
#include "sw_client.h"
#include "32x8.h"
#include "ota_update.h"

#include "html.h"
#include "favicon.h"
#include "routes.h"

IPAddress myIP;
String apIP;
//uint32_t printIP_ts  = 0;

String privet = "Hi, Cone-Man! MotoGymkhana the best!!!";

void printIP(void) {

  ssid_state = digitalRead(SSID_PIN);
  Brightness_less_State = digitalRead(BRIGHTNESS_less_PIN);
  if (ssid_state == LOW && Brightness_less_LastState != Brightness_less_State && Brightness_less_State == LOW) {
    apIP = "IP: ";
    myIP = WiFi.localIP();
    //################
    apIP += String(myIP[0]) + ".";
    apIP += String(myIP[1]) + ".";
    apIP += String(myIP[2]) + ".";
    apIP += String(myIP[3]);
    //###################
    shiftX = 0;
    //PrintTicker(apIP, blue, apIP.length());
    printIP_ts  = millis() + 25000;
    Brightness_less_LastState = Brightness_less_State;
  }
}

void setup() {
  pinMode(SSID_PIN, INPUT_PULLUP);
  pinMode(FONT_PIN, INPUT_PULLUP);       // пин кнопки переключения шрифта
  pinMode(BRIGHTNESS_less_PIN, INPUT_PULLUP);
  // pinMode(LAST_TIME_PIN, INPUT_PULLUP);  // пин кнопки переключения предыдущего времени
  pinMode(BRIGHTNESS_PIN, INPUT_PULLUP);

  InitConfig();

  if ( digitalRead(SSID_PIN) == LOW ) {
    wifi_id = 0;
    wifiMode = "server";
    ssid_laststate = LOW;
    if ( digitalRead(FONT_PIN) == LOW ) {
      saveDefaultConfigFile();
      InitConfig();
    }
  }
  InitDisplay();

  InitWifi();

  WiFi.onEvent(WiFiEvent);
  // event handler
  webSocket.onEvent(webSocketEvent);
  
  //connectToHost();
  
  InitRoutes();

  OTAWeb_update_begin();

  server.begin();
}

void loop() {
  webSocket.loop();

  matrix.fillScreen(0);
  TimerLoop();

  ssidChangeLoop();
  
  printIP();
  
  FontChangeLoop();

  //LastTimeIDChangeLoop();
  BrightnessChangeLoop();
  printWifiState();
  printWsState();
  // PrintTime();
  
  if (ssid_state_ts > millis() ) {
    PrintTicker("W:" + config["wifi"]["list"][wifi_id]["ssid"].as<String>(), NumbersColor, 4 + config["wifi"]["list"][wifi_id]["ssid"].as<String>().length());
  }
  else if (printIP_ts > millis() ) {
    PrintTicker(apIP, blue, apIP.length());
  }
  else if ( Brightness_State_ts > millis() ) {
    matrix.setTextColor(blue);
    matrix.setCursor(0, 0);
    matrix.print("Bri");
    matrix.setCursor(15, 0);
    matrix.print(".");
    matrix.setCursor(20, 0);
    matrix.print(String( Brightness / Brightness_change ));
    matrix.show();
  } 
  else {
    PrintTime();
    matrix.show();                              // Функция показа текста
}
  //TimePrintXY(LastTime[LastTimeID], 0, 40, "LastTime " + String(LastTimeID) + ": ");

  // matrix.show();
  delayMicroseconds(1);
}
