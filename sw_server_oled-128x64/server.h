//#include "WString.h"
IPAddress myIP;
String apIP = "";

// */

/* API is quite simple :
#include “esp_timer.h”

then call the function

int64_t esp_timer_get_time (void)

*/

bool ssid_state = HIGH;
bool ssid_laststate = HIGH;
uint32_t ssid_state_ts =  millis() ;

DNSServer dnsServer;
const char *server_name = "*"; //"sw";  // Can be "*" to all DNS requests

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void connectToHost() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin(config["wifi"]["list"][wifi_id]["ssid"].as<String>(), config["wifi"]["list"][wifi_id]["pass"].as<String>());
}

void StartAPMode() {
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(config["wifi"]["list"][wifi_id]["ssid"].as<String>(), config["wifi"]["list"][wifi_id]["pass"].as<String>(), 13);
}

void InitWifi(){
  if ( wifiMode == "server" ){
    StartAPMode();
  } else if ( wifiMode == "client") {
    connectToHost();
  }
}

void InitDNS(){
    dnsServer.start(53, server_name, IPAddress(192, 168, 4, 1));
}

void ssidChangeLoop() {
  button02_State = digitalRead(button02);
  button03_State = digitalRead(button03);
  if (button02_State == LOW && button02_LastState != button02_State && button03_State == LOW) {
   // ResetTime();
    if (wifi_id < wifiList.size() - 1) {
      wifi_id++;
    } else {
      wifi_id = 0;
    }
    wifiMode = wifiList[wifi_id]["mode"].as<String>();
    //delay(250);
    InitWifi();
    config["wifi"]["wifiid"] = String(wifi_id);
    saveConfig();
    // EEPROM.write(0, wifi_id);
    // EEPROM.commit();
    ssid_state_ts = millis();
    button02_LastState = button01_State;  
    }
  button03_LastState = button03_State; 

}

void notifyClients(void) {
  ws.textAll(startStopStateName + String(timerState) + String(currentTime));
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
