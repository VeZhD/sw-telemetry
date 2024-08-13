#ifndef DISPLAY_LASTTIME
#define DISPLAY_LASTTIME 1
#endif

#ifndef FONT_PIN
#define FONT_PIN 1
#endif

#ifndef SW_BASIC_OTA_HOSTNAME
#define SW_BASIC_OTA_HOSTNAME "SW_client"
#endif

//#ifndef SW_BASIC_OTA_PASSWORD
//#define SW_BASIC_OTA_PASSWORD "passwordSW_client"
//#endif

bool ssid_state = HIGH;
bool ssid_laststate = HIGH;
uint32_t ssid_state_ts =  millis() ;

bool Font_State = HIGH;
bool Font_LastState = HIGH;

WebSocketsClient webSocket;

//unsigned long messageInterval = 5000;
bool Connected = false;
bool ConnectedWS = false;

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
      TimerState = payload_str.substring(1, 2).toInt();
      CurrentTime = payload_str.substring(2).toInt();
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

#if defined(ESP32)
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
      if (wsSSL[wifi_id] == true) {
        webSocket.beginSSL(wsHost[wifi_id], wsPort[wifi_id], wsPath[wifi_id]);
      } else {
        webSocket.begin(wsHost[wifi_id], wsPort[wifi_id], wsPath[wifi_id]);
      }
      break;
  }
}
#endif

#if defined(ESP8266)
void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case WIFI_EVENT_STAMODE_CONNECTED:
      //Serial.println("WIFI_EVENT_STAMODE_CONNECTED");
      Connected = true;
      break;
    case WIFI_EVENT_STAMODE_AUTHMODE_CHANGE:
      //Serial.println("WIFI_EVENT_STAMODE_AUTHMODE_CHANGE");
      break;
    case WIFI_EVENT_STAMODE_GOT_IP:
      //Serial.println("WIFI_EVENT_STAMODE_GOT_IP");
      //Serial.println(WiFi.localIP().toString());
      if (wsSSL[wifi_id] == true) {
        webSocket.beginSSL(wsHost[wifi_id], wsPort[wifi_id], wsPath[wifi_id]);
      } else {
        webSocket.begin(wsHost[wifi_id], wsPort[wifi_id], wsPath[wifi_id]);
      }
      break;
    case WIFI_EVENT_STAMODE_DISCONNECTED:
      //Serial.println("WIFI_EVENT_STAMODE_DISCONNECTED");
      Connected = false;
      break;
    case WIFI_EVENT_SOFTAPMODE_PROBEREQRECVED:
      break;
  }
}
#endif

void connectToHost() {
  WiFi.disconnect(true);
  WiFi.begin(ssid[wifi_id], password[wifi_id]);
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
  Font_State = digitalRead(FONT_PIN);
  if (ssid_state == LOW && Font_LastState != Font_State && Font_State == LOW) {
  //if (ssid_state == LOW && ssid_laststate == HIGH) {
    if (wifi_id < (sizeof(ssid) / sizeof(char *) - 1) && wifi_id >= 0) {
      wifi_id++;
    } else {
      wifi_id = 0;
    }
    EEPROM.write(0, wifi_id);
    EEPROM.commit();
    ssid_state_ts = millis();
    ResetTime();
    delay(250);
    connectToHost();
    Font_LastState = Font_State;
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