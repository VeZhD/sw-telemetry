/*********
Connect I2C Oled display 
SCL -> WeMos D1 mini - D1(GPIO5) / Lolin s2 mini - 35
SDA -> WeMos D1 mini - D2(GPIO4) / Lolin s2 mini - 33
VCC -> 5V
GND -> GND
*********/

#define SENSOR_NPN // при использовании сенсора с NPN укзать SENSOR_NPN, при использовании сенсора с PNP укзать SENSOR_PNP
#define SENSOR_PIN 6     // пин подключения датчика луча


#define button01    10
#define button02    12
#define button03    13
#define HotPlug_pin 11

#include <WiFi.h>
#include <WiFiClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <DNSServer.h>
#include <esp_timer.h>

/* API is quite simple :
#include “esp_timer.h”

then call the function

int64_t esp_timer_get_time (void)

// Import required libraries for ESP8266
 /*
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <DNSServer.h>
// */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "SSID_server.h"
#include "html.h"
//#include "server.h"
//#include "128x64.h"

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

DNSServer dnsServer;
const char *server_name = "*"; //"sw";  // Can be "*" to all DNS requests

bool  HotPlug_State;
bool  HotPlug_LastState = HIGH;

bool ledState = 0;
const int ledPin = 2;

int startStopState;
String startStopStateName;
String apIP = "";
int startStopLastState;
int timerState = 0;
long startTime;
long elapsedTime;
long currentTime = 0;
long lastWsUpTime = 0;
long lastChange;
int minutes = 0;
int tSeconds = 0;
int seconds = 0;
int mSeconds = 0;
int miSeconds = 0;
int milSeconds = 0;

int StopDelay = 2500; // задержка срабатывания на луч в миллисекундах

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void printtime(void) {
  minutes = (int)(currentTime / 60000) % 10;
  tSeconds = (int)(currentTime % 60000 / 10000);
  seconds = (int)(currentTime % 60000 % 10000 / 1000);
  mSeconds = (int)(currentTime % 60000 % 1000 / 100);
  miSeconds = (int)(currentTime % 60000 % 100 / 10);
  milSeconds = (int)(currentTime % 60000 % 10);

  display.clearDisplay();

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


  display.setTextSize(1);  // Draw 1X-scale text
  display.println("#####################");
  //display.println(String(HotPlug_State) + "  " + String(HotPlug_LastState));
  display.println("IPv4: " + apIP);
  display.println("Wifi: " + String(ssid_name));
  display.println("Pass: " + String(ssid_pass));

  if (startStopState == LOW) {
    display.setTextColor(SSD1306_WHITE);
    display.println("Sensor is working! ");
  } else {

    display.setTextColor(SSD1306_WHITE);
    display.println("Sensor error!!! ");
  }

  display.display();
}

void printip(void) {
  display.clearDisplay();
  display.setCursor(1, 32);  // Start at top-left corner
  display.setTextSize(1);    // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.println("Made by VeZhD");
  //display.println("VeZhD");
  display.println("Code from Alekssaff");
  display.display();
  delay(1500);
}

void notifyClients(void) {
  lastWsUpTime = millis();
  if (startStopState == LOW) {
    startStopStateName = '1';
  } else {
    startStopStateName = '0';
  }
  ws.textAll(startStopStateName + String(timerState) + String(currentTime));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char *)data, "toggle") == 0) {
      ledState = !ledState;
      notifyClients();
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
//      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
//      Serial.printf("WebSocket client #%u disconnected\n", client->id());
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

void HotPlug_display() {
  HotPlug_State = digitalRead(HotPlug_pin);
  if (HotPlug_State == LOW && HotPlug_LastState != HotPlug_State) {
    delay(1500);
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  }

  HotPlug_LastState = HotPlug_State;  
}

void setup() {
#if defined(SENSOR_NPN)
  pinMode(SENSOR_PIN, INPUT_PULLUP);
#elif defined(SENSOR_PNP)
  pinMode(SENSOR_PIN, INPUT);
#else
#error "The type of sensor used is not specified"
#endif

  pinMode(HotPlug_pin, INPUT_PULLUP);

//  Serial.begin(115200);
//  Serial.println();
//  Serial.println("Configuring access point...");

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
//    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  display.display();
  delay(1000); 

  // You can remove the password parameter if you want the AP to be open.(ssid, password,channel, hide=1, clients max= )
  //WiFi.softAP(ssid_name, ssid_pass, 13, 1, 5);
  WiFi.softAP(ssid_name, ssid_pass, 13);

  IPAddress myIP = WiFi.softAPIP();

  const byte DNS_PORT = 53;
  dnsServer.start(DNS_PORT, server_name, myIP);

  //################
  apIP += String(myIP[0]) + ".";
  apIP += String(myIP[1]) + ".";
  apIP += String(myIP[2]) + ".";
  apIP += String(myIP[3]);
  //###################
//  Serial.print("AP IP address: ");
//  Serial.println(myIP);

  printip();

  // А нужна ли эта строчка?
  //sserver.begin();

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  AsyncElegantOTA.begin(&server,web_user,web_pass);    // Start AsyncElegantOTA
  // Start server
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  
  HotPlug_display();

#if defined(SENSOR_NPN)
  startStopState = !digitalRead(SENSOR_PIN);
#elif defined(SENSOR_PNP)
  startStopState = digitalRead(SENSOR_PIN);
#else
#error "The type of sensor used is not specified"
#endif

  if (timerState == 0) {
    if (startStopState == HIGH && startStopLastState == LOW && millis() - lastChange > StopDelay) {
      startTime = millis();
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
      elapsedTime = currentTime;
      lastChange = millis();
      notifyClients();
    }
    startStopLastState = startStopState;
  }
  //  if (millis() % 60000 == 0){
  //    ws.cleanupClients();
  //  }
  if (timerState == 1 && millis() - 750 > lastWsUpTime) {
    notifyClients();
  }
  if (timerState == 0 && millis() - 1000 > lastWsUpTime) {
    notifyClients();
  }
  printtime();
  //delay(1);
}
