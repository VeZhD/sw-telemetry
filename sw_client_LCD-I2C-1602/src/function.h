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
      Serial.println("disconnected ws");
      break;
    case WStype_CONNECTED:
      {
        ConnectedWS = true;
        Serial.println("Connected WS");
        webSocket.sendTXT("Connected");
      }
      break;
    case WStype_TEXT:
      StartStopState = payload_str.substring(0, 1).toInt();
      TimerState = payload_str.substring(1, 2).toInt();
      CurrentTime = payload_str.substring(2).toInt();
      StartTime = millis() - CurrentTime;
      Serial.println(payload_str);
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
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;
  }
}

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_READY:
      Serial.println("ESP32 WiFi ready");
      break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
      Serial.println("ESP32 finish scanning AP");
      break;
    case ARDUINO_EVENT_WIFI_STA_START:
      Serial.println("ESP32 station start");
      break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
      Serial.println("ESP32 station stop");
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.println("wifi connected");
      Connected = true;
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("wifi disconnected");
      Connected = false;
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.println("Obtained IP address: ");
      Serial.println(WiFi.localIP());
      Serial.println(WiFi.gatewayIP());
      if (wsSSL[wifi_id] == true) {
        webSocket.beginSSL(wsHost[wifi_id], wsPort[wifi_id], wsPath[wifi_id]);
      } else {
        webSocket.begin(wsHost[wifi_id], wsPort[wifi_id], wsPath[wifi_id]);
      }
      break;
  }
}

void connectToHost() {
  WiFi.disconnect(true);
  WiFi.begin(ssid[wifi_id], password[wifi_id]);
  // server address, port and URL
  Serial.println("connecting wifi...");
  Serial.println(ssid[wifi_id]);
  Serial.println(password[wifi_id]);
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
}

void ssidChangeLoop() {
  ssid_state = digitalRead(SSID_PIN);
  if (ssid_state == LOW && ssid_laststate == HIGH) {
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
  }
  ssid_laststate = ssid_state;
}

void TimerLoop() {
  if (TimerState == HIGH && TimerLastState != TimerState) {
    for (int i = LastTimeCount - 1; i > 0; i--) {
      LastTime[i] = LastTime[i - 1];
    }
    LastTime[0] = LastCurrentTime;

    StartTime = millis();
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
