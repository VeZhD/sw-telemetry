//#include <cstddef>
#ifndef DISPLAY_LASTTIME
#define DISPLAY_LASTTIME 1
#endif

bool ssid_state = HIGH;
bool ssid_laststate = HIGH;
uint32_t ssid_state_ts = 0 ;

bool Font_State = HIGH;
bool Font_LastState = HIGH;

WebSocketsClient webSocket;

bool Connected = false;
bool ConnectedWS = false;

// IPAddress myIP;
// String apIP;
uint32_t printIP_ts  = 0;

uint32_t StartTime = 0;
uint32_t CurrentTime = 0;
bool StartStopState = 0;
bool TimerState;
bool TimerLastState;

uint8_t CurrentMinutes = 0;
uint8_t CurrenttSeconds = 0;
uint8_t Currentseconds = 0;
uint8_t CurrentmSeconds = 0;
uint8_t CurrentmiSeconds = 0;
uint8_t CurrentmilSeconds = 0;

const uint8_t LastTimeCount = 100;
uint32_t LastTime[LastTimeCount];
uint32_t LastCurrentTime = 0;
uint8_t LastTimeID = 0;
bool LastTimeID_State = HIGH;
bool LastTimeID_LastState = HIGH;
uint32_t LastTimeID_State_ts = millis();

uint32_t TopTime = 599999;
uint count = 0;

int StartStopLastState;

void connectToHost() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin(config["wifi"]["list"][wifi_id]["ssid"].as<String>(), config["wifi"]["list"][wifi_id]["pass"].as<String>());
  //WiFi.begin(ssid[wifi_id], password[wifi_id]);
}

void StartAPMode() {
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(\
    // SSID Name
    config["wifi"]["list"][wifi_id]["ssid"].as<String>(), \
    // SSID Password
    config["wifi"]["list"][wifi_id]["pass"].as<String>(), \
    // Wi-Fi Channel 
    config["wifi"]["list"][wifi_id]["ssid_channel"].as<uint8_t>(), \
    // Wi-Fi Hidenn: 0 - видно, 1 - скрыто
    config["wifi"]["list"][wifi_id]["ssid_hidden"].as<uint8_t>(), \
    // max_connection: 1-10, рекомендуетя 4-6, чем меньше, тем стабильнее подключение
    config["wifi"]["list"][wifi_id]["max_connection"].as<uint8_t>()); 
  //WiFi.softAP(ssid[wifi_id], password[wifi_id], 13); 
}

void InitWifi(){

  if ( wifiMode == "server" ){
    webSocket.disconnect();
    StartAPMode();
  } else if ( wifiMode == "client") {
    connectToHost();
  }
}

void SetTime() {
  CurrentMinutes = (int)(CurrentTime / 60000) % 10;
  CurrenttSeconds = (int)(CurrentTime % 60000 / 10000);
  Currentseconds = (int)(CurrentTime % 60000 % 10000 / 1000);
  CurrentmSeconds = (int)(CurrentTime % 60000 % 1000 / 100);
  CurrentmiSeconds = (int)(CurrentTime % 60000 % 100 / 10);
  CurrentmilSeconds = (int)(CurrentTime % 60000 % 10);
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  String payload_str = String((char *)payload);

  switch (type) {
    case WStype_DISCONNECTED:
      ConnectedWS = false;
      break;
    case WStype_CONNECTED:
      {
        ConnectedWS = true;
        webSocket.sendTXT("Connected");
      }
      break;
    case WStype_TEXT:
      StartStopState = payload_str.substring(0, 1).toInt();
      if ( config["wifi"]["list"][wifi_id]["ssid"].as<String>().substring(0, 20) == "MOTO_TRAINING_SINGLE" ) {
        TimerState = payload_str.substring(10, 11).toInt();
        CurrentTime = payload_str.substring(21,27).toInt();
      } else {
        TimerState = payload_str.substring(1, 2).toInt();
        CurrentTime = payload_str.substring(2).toInt();
      }

      StartTime = millis() - CurrentTime;
      SetTime();
      break;
    case WStype_BIN:
      break;
    case WStype_PING:
      break;
    case WStype_PONG:
      // answer to a ping we send
      break;
    case WStype_ERROR:
      break;
    case WStype_FRAGMENT_TEXT_START:
      break;
    case WStype_FRAGMENT_BIN_START:
      break;
    case WStype_FRAGMENT:
      break;
    case WStype_FRAGMENT_FIN:
      break;
  }
}

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_READY:
      break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
      break;
    case ARDUINO_EVENT_WIFI_STA_START:
      break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Connected = true;
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Connected = false;
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:

      // if ( wifiMode == "server" ){
      //   webSocket.disconnect();
      //   break;
      // }

      if ( config["wifi"]["list"][wifi_id]["ssid"].as<String>().substring(0, 20) == "MOTO_TRAINING_SINGLE" ) {
        webSocket.begin("10.10.10.100", 89, "/");
      } else if ( config["wifi"]["list"][wifi_id]["wsPort"].as<int>() == 8000 ) {
        webSocket.beginSSL(config["wifi"]["list"][wifi_id]["wsHost"].as<String>(), config["wifi"]["list"][wifi_id]["wsPort"].as<int>(), config["wifi"]["list"][wifi_id]["wsPath"].as<String>());
      } else  if ( config["wifi"]["list"][wifi_id]["wsPort"].as<int>() == 80 ){
        webSocket.begin(config["wifi"]["list"][wifi_id]["wsHost"].as<String>(), config["wifi"]["list"][wifi_id]["wsPort"].as<int>(), config["wifi"]["list"][wifi_id]["wsPath"].as<String>());
      } else {
         webSocket.begin("192.168.4.1", 80, "/ws");
      }

      // if (config["wifi"]["list"][wifi_id]["wsSSL"].as<bool>() == true) {
      //   webSocket.beginSSL(config["wifi"]["list"][wifi_id]["wsHost"].as<String>(), config["wifi"]["list"][wifi_id]["wsPort"].as<int>(), config["wifi"]["list"][wifi_id]["wsPath"].as<String>());
      // } else {
      //   webSocket.begin(config["wifi"]["list"][wifi_id]["wsHost"].as<String>(), config["wifi"]["list"][wifi_id]["wsPort"].as<int>(), config["wifi"]["list"][wifi_id]["wsPath"].as<String>());
      // }
     
      // webSocket.begin(wsHost[wifi_id], wsPort[wifi_id], wsPath[wifi_id]);
      
      break;
  }
}

void CalcTopTime() {
  if ((1500 < CurrentTime) && (CurrentTime < TopTime) && (CurrentTime > uint32_t(TopTime / 1.15) or TopTime == 599999) && (ssid_state_ts < millis() - 7500)) {
    TopTime = CurrentTime;
  }
  count++;
}

void ResetTime() {
  StartTime = 0;
  CurrentTime = 0;
  StartStopState = 0;
  TimerState = LOW;
  TimerLastState = LOW;
  TopTime = 599999;
  
  for (int i = LastTimeCount - 1; i >= 0; i--) {
    LastTime[i] = 0;
  }

}

void ssidChangeLoop() {
  ssid_state = digitalRead(SSID_PIN);

#ifdef ws2811
  Font_State = digitalRead(FONT_PIN);
  if ( wifiList.size() - 1 !=0  && ssid_state == LOW && Font_LastState != Font_State && Font_State == LOW) {
#endif

#ifndef ws2811
  if ( wifiList.size() - 1 !=0  && ssid_state == LOW && ssid_laststate == HIGH) {
#endif

    // if (wifi_id < (sizeof(ssid) / sizeof(char *) - 1) && wifi_id >= 0) {
    //   wifi_id++;
    // } else {
    //   wifi_id = 0;
    // }
    if (wifi_id < wifiList.size() - 1) {
      wifi_id++;
    } else {
      wifi_id = 1;

    }
    wifiMode = wifiList[wifi_id]["mode"].as<String>();
    ResetTime();
    // delay(250);
    // connectToHost();
    InitWifi();
    config["wifi"]["wifiid"] = String(wifi_id);
    saveConfig();
    ssid_state_ts = millis() + 13500;

#ifdef ws2811
    Font_LastState = Font_State;
    shiftX = 0;
#endif

  }
  ssid_laststate = ssid_state;
}

void TimerLoop() {
  if (TimerState == HIGH && TimerLastState != TimerState) {
    for (int i = LastTimeCount - 1; i > 0; i--) {
      LastTime[i] = LastTime[i - 1];
    }
    LastTime[0] = LastCurrentTime;

    //StartTime = millis();
    CurrentTime = millis() - StartTime;
    TimerLastState = TimerState;
  } else if (TimerState == HIGH) {
    CurrentTime = millis() - StartTime;
  } else {
    LastCurrentTime = CurrentTime;
    CalcTopTime();
  }
  if (TimerLastState != TimerState) {
    TimerLastState = TimerState;
  }
  SetTime();
}

#ifdef LAST_TIME_PIN
void get_IP(void) {

  ssid_state = digitalRead(SSID_PIN);
  LastTimeID_State = digitalRead(LAST_TIME_PIN);
  // if (ssid_state == LOW && LastTimeID_State != LastTimeID_LastState && LastTimeID_State == LOW) {
  if (ssid_state == LOW && LAST_TIME_PIN == LOW && ssid_state != ssid_laststate ) {
    // apIP = "IP: ";
    // myIP = WiFi.localIP();
    // //################
    // apIP += String(myIP[0]) + ".";
    // apIP += String(myIP[1]) + ".";
    // apIP += String(myIP[2]) + ".";
    // apIP += String(myIP[3]);
    //###################
    #ifdef ws2811
    shiftX = 0;
    #endif
    printIP_ts  = millis() + 25000;
    ssid_state_ts = millis() + 25000;
    LastTimeID_LastState = LAST_TIME_PIN;
    ssid_laststate = ssid_state;
  }
}

void LastTimeIDChangeLoop() {
  LastTimeID_State = digitalRead(LAST_TIME_PIN);
  if (LastTimeID_State == LOW && LastTimeID_LastState == HIGH) {
    if ( LastTimeID >= 0 && LastTimeID < LastTimeCount - DISPLAY_LASTTIME && LastTime[LastTimeID + DISPLAY_LASTTIME] > 1 ) {
      LastTimeID++;
    } else {
      LastTimeID = 0;
    }
    LastTimeID_State_ts = millis();
  }
  
  LastTimeID_LastState = LastTimeID_State;
  
  if (LastTimeID_State_ts + 10500 < millis()) {
    LastTimeID = 0;
  }
}
#endif

#ifdef FONT_PIN
void FontChangeLoop() {

  Font_State = digitalRead(FONT_PIN);
  if (Font_State == LOW && Font_LastState == HIGH) {
    if (Font_ID >= 0 && Font_ID < Font_Count) {
      Font_ID++;
    } else {
      Font_ID = 0;
    }
    config["timer"]["font_id"] = Font_ID;
    saveConfig();
  // #ifdef OLED_I2C
  //   myOLED.clrScr();
  // #endif
  }
  Font_LastState = Font_State;
}
#endif

#ifdef OTA_update_ENABLED
void SW_Basic_OTA() {
  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname(SW_BASIC_OTA_HOSTNAME);
  // No authentication by default
  //ArduinoOTA.setPassword();
  ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";
    });

  ArduinoOTA.begin();
}
#endif
