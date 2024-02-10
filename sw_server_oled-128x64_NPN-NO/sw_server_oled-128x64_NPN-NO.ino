
/*********
Connect I2C Oled display 
SCL -> WeMos D1 mini - D1(GPIO5) / Lolin s2 mini - 35
SDA -> WeMos D1 mini - D2(GPIO4) / Lolin s2 mini - 33
VCC -> 5V
GND -> GND
*********/

// Import required libraries for ESP32/ESP32s2/etc.
// /*
#include <WiFi.h>
#include <WiFiClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <DNSServer.h>
// */

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

// from src path:
#include "src/SSID_server.h"
//#include "src/128x64.h"
#include "src/server_variables.h"

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

DNSServer dnsServer;
const char *server_name = "*"; //"sw";  // Can be "*" to all DNS requests

bool ledState = 0;
const int ledPin = 2;

#define SENSOR_PIN 6     // пин подключения датчика луча
#define LAST_TIME_PIN 39  // пин кнопки переключения предыдущего времени(1-10)

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
  //display.fillRect(121, 0, 2, 15, SSD1306_WHITE); // Draw !
  //display.fillRect(120, 18, 4, 3, SSD1306_WHITE); // Draw !

  display.setTextSize(1);  // Draw 1X-scale text
  display.println("#####################");
  // display.println("---------------------");
  display.println("IPv4: " + apIP);
  display.println("Wifi: " + String(ssid));
  display.println("Pass: " + String(password));

  if (startStopState == LOW) {
    display.setTextColor(SSD1306_WHITE);
    display.println("Sensor is working! ");
    //    display.fillRect(0, 59, 128, 61, SSD1306_BLACK);
  } else {

    display.setTextColor(SSD1306_WHITE);
    display.println("Sensor error!!! ");
    //    display.fillRect(0, 56, 128, 61, SSD1306_WHITE);
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
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket(void) {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup() {
  // Serial port for debugging purposes
  pinMode(SENSOR_PIN, INPUT);

  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  display.display();
  delay(2000);  // Pause for 2 seconds

  // You can remove the password parameter if you want the AP to be open.(ssid, password,channel, hide=1, clients max= )
  //WiFi.softAP(ssid, password, 13, 1, 5);
  WiFi.softAP(ssid, password, 13);

  IPAddress myIP = WiFi.softAPIP();

  const byte DNS_PORT = 53;
  dnsServer.start(DNS_PORT, server_name, myIP);

  //################
  apIP += String(myIP[0]) + ".";
  apIP += String(myIP[1]) + ".";
  apIP += String(myIP[2]) + ".";
  apIP += String(myIP[3]);
  //###################
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  printip();

  server.begin();

  Serial.println("Server started");

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  // Start server
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();

  startStopState = digitalRead(SENSOR_PIN);

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
  if (timerState == 1 && millis() - 1500 > lastWsUpTime) {
    notifyClients();
  }
  if (timerState == 0 && millis() - 2000 > lastWsUpTime) {
    notifyClients();
  }
  printtime();
  //delay(1);
}
