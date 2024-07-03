/*********
Connect I2C Oled display 
SCL -> WeMos D1 mini - D1(GPIO5) / Lolin s2 mini - 35 / Lolin s3 mini - 36
SDA -> WeMos D1 mini - D2(GPIO4) / Lolin s2 mini - 33 / Lolin s3 mini - 35
VCC -> 5V
GND -> GND
*********/

//#define SENSOR_NPN // при использовании сенсора с NPN укзать SENSOR_NPN, при использовании сенсора с PNP укзать SENSOR_PNP
//#define SENSOR_NO // при использовании сенсора с NO(Normal Open, нормально открытый) укзать SENSOR_NO, при использовании сенсора NC(Normal Closed, нормально закрытый) укзать SENSOR_NC

#define SENSOR_PIN 6     // пин подключения датчика луча

#define button01    13
#define button02    12
#define button03    10
#define HotPlug_pin 11

#define EEPROM_SIZE   64    // размер EEPROM

#if defined(ESP32)
  #pragma message "ESP32 stuff happening!"

  #include <WiFi.h>
  #include <AsyncTCP.h>
  #include <Update.h>

#elif defined(ESP8266)
//С ESP8266 не было полноценных тестов, возможно что-то отвалится, обязательно проверить используемые пины
  #pragma message "ESP8266 stuff happening!"

  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  
  #undef button01
  #undef button02
  #undef button03
  #undef SENSOR_PIN
  #undef HotPlug_pin

  #define SENSOR_PIN 2

  #define button01    12
  #define button02    13
  #define button03    14
  #define HotPlug_pin 14
#else
  #error "This ain't a ESP8266 or ESP32, dumbo!"
#endif

#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <DNSServer.h>
#include <EEPROM.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPIFFS.h>

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

DNSServer dnsServer;
const char *server_name = "*"; //"sw";  // Can be "*" to all DNS requests

uint PrintDelay = 10000;  // задержка отображения результата в режиме кругового таймера
uint StopDelay = 1650;    // задержка срабатывания на луч в миллисекундах

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

#include "SSID_server.h"
#include "html_ws.h"
#include "html_get.h"
#include "favicon.h"
#include "script.js.gz.h"
#include "NoSleep.min.js.gz.h"
#include "128x64.h"
#include "server.h"
#include "ota_update.h"

void printtime(long time) {
  int minutes = (int)(time / 60000) % 10;
  int tSeconds = (int)(time % 60000 / 10000);
  int seconds = (int)(time % 60000 % 10000 / 1000);
  int mSeconds = (int)(time % 60000 % 1000 / 100);
  int miSeconds = (int)(time % 60000 % 100 / 10);
  int milSeconds = (int)(time % 60000 % 10);

  display.clearDisplay();

  if (Font_ID == Font_Count) {

    display.setCursor(0, 0);  // Start at top-left corner

    display.setTextSize(3);  // Draw 3X-scale text
    display.setTextColor(SSD1306_WHITE);
    display.print(minutes);
    display.setTextSize(1);  // Draw 1X-scale text
    display.print(F(" "));
    display.fillRect(18, 6, 3, 3, SSD1306_WHITE);   // Draw :
    display.fillRect(18, 12, 3, 3, SSD1306_WHITE);  // Draw :
    display.setTextSize(3);                         // Draw 3X-scale text
    display.print(tSeconds);
    display.print(seconds);
    display.setTextSize(1);  // Draw 1X-scale text
    display.print(F(" "));
    display.fillRect(60, 18, 3, 3, SSD1306_WHITE);  // Draw .
    display.setTextSize(3);                         // Draw 3X-scale text
    display.print(mSeconds);
    display.print(miSeconds);
    display.println(milSeconds);
    display.setTextWrap(false);
    display.setTextSize(1);
    display.println("Wifi: " + String(ssid_name[wifi_id]));
    if ( wifi_id == 0) {
      display.println("Pass:" + String(ssid_pass[0]));
    } else {
      myIP = WiFi.localIP();
      //################
      apIP = String(myIP[0]) + ".";
      apIP += String(myIP[1]) + ".";
      apIP += String(myIP[2]) + ".";
      apIP += String(myIP[3]);
      //###################
      display.print("IPv4: " + apIP);
    }

  } else {
    int dX = 1;
    // Minutes
    drawDigit(dX + 0, minutes);
    // Seconds
    drawDigit(dX + (1 * SizeX) + 5, tSeconds);
    drawDigit(dX + (2 * SizeX) + 6, seconds);
    // milliseconds
    drawDigit(dX + (3 * SizeX) + 11, mSeconds);
    drawDigit(dX + (4 * SizeX) + 12, miSeconds);
    drawDigit(dX + (5 * SizeX) + 13, milSeconds);
    // Двоеточие
    display.fillRect(dX + (1 * SizeX) + 1, int(SizeY / 3), 3, 3, SSD1306_WHITE);
    display.fillRect(dX + (1 * SizeX) + 1, int(SizeY - SizeY / 3), 3, 3, SSD1306_WHITE);
    // Точка
    display.fillRect(dX + (3 * SizeX) + 7, SizeY - 3, 3, 3, SSD1306_WHITE);
  }

  display.setCursor(20, 40);
  display.print("mode: " + modeString);

  display.setCursor(0, 56);
  if (startStopState == LOW) {
    display.setTextColor(SSD1306_WHITE);
    display.print("Sensor is working! ");
  } else {
    display.setTextColor(SSD1306_WHITE);
    display.print("Sensor error!!! ");
  }
  
}

void printip(void) {
  display.clearDisplay();
  display.setCursor(1, 32);  // Start at top-left corner
  display.setTextSize(1);    // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.println("Made by VeZhD");
  display.println("Code from Alekssaff");
  display.display();
  delay(1500);
}

void notifyClients(void) {
  ws.textAll(String(!startStopState) + String(timerState) + String(currentTime));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char *)data, "toggle") == 0) {
      notifyClients();
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:  //      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT: //      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
      break;
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket(void) {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

/*
boolean find_i2c(uint8_t address) { // функция проверки устройства по указанному адресу I2C
  Wire.beginTransmission(address);
  return (Wire.endTransmission () == 0); //возвращает true если ошибок нет (устройство подключено и отвечает)
}
*/

void setup() {

#if defined(SENSOR_NPN)
  pinMode(SENSOR_PIN, INPUT_PULLUP);
#elif defined(SENSOR_PNP)
  pinMode(SENSOR_PIN, INPUT);
#else
#error "The type of sensor used is not specified"
#endif

  pinMode(HotPlug_pin, INPUT_PULLUP);

  pinMode(button01, INPUT_PULLUP);
  pinMode(button02, INPUT_PULLUP);
  pinMode(button03, INPUT_PULLUP);

#if defined(ESP32)
  // For ESP32/ESP32s2/ESP32s3
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
  if ((EEPROM.read(0) >= 0) && (EEPROM.read(0) < (sizeof(ssid_name) / sizeof(char *)))) {
    wifi_id = EEPROM.read(0);
  }

//  Serial.begin(115200);
//  Serial.println();
//  Serial.println("Configuring access point...");

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.display();
  delay(150); 

  /*
  // если подключен дисплей - инициализируем его
  if( display_enable = find_i2c(SCREEN_ADDRESS) ) {
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);    
    display.clearDisplay();
    display.display();
    delay(300); 
    }
*/

  if ( wifi_id == 0) {
    StartAPMode();
  } else {
    connectToHost();
  }

  dnsServer.start(53, server_name, IPAddress(192, 168, 4, 1));

  printip();

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
    // < or >
    // AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", html_get_h_gz, html_get_h_gz_len);
    // response->addHeader("Content-Encoding", "gzip");
    // request->send(response);
  });

  server.on("/ws", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html_ws);
    // < or >
    // AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", html_ws_h_gz, html_ws_h_gz_len);
    // response->addHeader("Content-Encoding", "gzip");
    // request->send(response);
  });

  server.on("/laptime", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", laptime);
  });

  server.on("/favicon.ico",  HTTP_GET, [](AsyncWebServerRequest *request){    
    // AsyncWebServerResponse *response = request->beginResponse_P(200, "image/x-icon", favicon_ico, favicon_ico_len);
    // AsyncWebServerResponse *response = request->beginResponse_P(200, "image/x-icon", favicon_ico_gz, favicon_ico_gz_len);
    // response->addHeader("Content-Encoding", "gzip");
    // request->send(response);
    // < or >
    request->send_P(200, "image/png", favicon, favicon_png_len);
    });

  server.on("/NoSleep.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    // request->send_P(200, "text/javascript", NoSleep_min_js);
    // < or >
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript", NoSleep_min_js_gz, NoSleep_min_js_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    // request->send_P(200, "text/javascript", script_js_gz);
    // < or >
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript", script_js_gz, script_js_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  //OTAWeb_update_begin();
  AsyncElegantOTA.begin(&server,web_user,web_pass);    // Start AsyncElegantOTA
  // Start server
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  
  HotPlug_display();
/*
if( display_enable ) {
  //display.clear();
  //display.setFont(ArialMT_Plain_16);
  //display.drawString(0, 0, "Test:");
  //display.display();
  display_enable = find_i2c(SCREEN_ADDRESS); // проверяем связь с дисплеем, чтобы отработать его отключение
  }
else  {
   // если дисплей отключить, а потом снова включить - он нуждается в командах инициализации
  if( display_enable = find_i2c(SCREEN_ADDRESS) ) {
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);    
    display.clearDisplay();
    display.display();
    }
  } 
*/

#if ( defined(SENSOR_NPN) and defined(SENSOR_NO) ) or ( defined(SENSOR_PNP) and defined(SENSOR_NC) )
  startStopState = !digitalRead(SENSOR_PIN);
#elif ( defined(SENSOR_PNP) and defined(SENSOR_NO) ) or ( defined(SENSOR_NPN) and defined(SENSOR_NC) )
  startStopState = digitalRead(SENSOR_PIN);
#else
#error "The type of sensor used is not specified"
#endif

switch (mode)
{
  case 0: // Start-Stop timer
    if (timerState == 0) {
      if (startStopState == HIGH && startStopLastState == LOW && millis() - lastChange > StopDelay) {
        startTime = millis();

        for (int i = LastTimeCount - 1; i > 0; i--) {
          LastTime[i] = LastTime[i - 1];
        }
        LastTime[0] = currentTime;

       timerState = 1;
       currentTime = millis() - startTime;
       lastChange = millis();
       notifyClients();
     }
     startStopLastState = startStopState;
    } else {
       currentTime = millis() - startTime;
     if (startStopState == HIGH && startStopLastState == LOW && millis() - lastChange > StopDelay) {
       timerState = 0;
       lastChange = millis();
       notifyClients();
     }
     startStopLastState = startStopState;
    }
    break;

  case 1:  // Срабатывание таймера по каждому пересечению луча, как китайский лаптаймер
    if (startStopState == HIGH && startStopLastState == LOW && millis() - lastChange > StopDelay) {
      currentTime = millis() - startTime;
      startTime = millis();

      for (int i = LastTimeCount - 1; i > 0; i--) {
        LastTime[i] = LastTime[i - 1];
      }
      LastTime[0] = currentTime;

      timerState = 0;
      notifyClients();
      lastChange = millis();
    }
    startStopLastState = startStopState;
  
    if (millis() - startTime >= PrintDelay) {
      currentTime = millis() - startTime;
      timerState = 1;
      notifyClients();
    }
    break;
}

  laptime = String(!startStopState) + String(timerState) + String(currentTime);

  //if (millis() - 1555 > lastWsUpTime) {
  //  notifyClients();
  //}  

  ChangeMode();
  ssidChangeLoop();

  LastTimeIDChangeLoop();
  FontChangeLoop();

  printtime(currentTime);
  if (LastTimeID <10) {
    TimePrintXY(LastTime[LastTimeID], 0, 48, "LastTime 0" + String(LastTimeID) + ": ");
    }
  else {
    TimePrintXY(LastTime[LastTimeID], 0, 48, "LastTime " + String(LastTimeID) + ": ");
  }

  TestPressButton01();
  TestPressButton02();
  TestPressButton03();
  
  display.display();
}
