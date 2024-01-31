/*********
display OLED i2c 128*64 witch WEMOS(LOLIN) S2 mini
SCL -> D1(GPIO5) For ESP8266(Wemos D1 mini) / 35 For ESP32s2(Wemos(Lolin) S2 mini)
SDA -> D2(GPIO4) For ESP8266(Wemos D1 mini) / 33 For ESP32s2(Wemos(Lolin) S2 mini)
VCC -> 5V
GND -> GND
*********/
// For ESP32\ESP32s2 and etc.
#include <WiFi.h>
// For ESP8266
//#include <ESP8266WiFi.h>

#include <EEPROM.h>
#include <WebSocketsClient.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

#define SSID_PIN      34    // пин кнопки переключения wifi сети
#define LAST_TIME_PIN 38    // пин кнопки переключения предыдущего времени(1-10)

#define EEPROM_SIZE 64

// настройки точки доступа
const char *ssid[]      = { "StopWatcher", "StopWatcher02" };   // список возможных сетей
const char *password[]  = { "StopWatcher", "StopWatcher02" };   // список паролей к сетям
const bool wsSSL[]      = { false, false };                     // если вебсокеты работают через SSL - true, обычно false
const char *wsHost[]    = { "192.168.4.1", "192.168.4.1" };     // адрес таймера
const char *wsPath[]    = { "/ws", "/ws" };                     // путь к вебсокетам
const int wsPort[]      = { 80, 80 };                           // порт вебсокетов
uint8_t wifi_id = 0;

bool ssid_state = HIGH;
bool ssid_laststate = HIGH;
uint32_t ssid_state_ts = millis();

WebSocketsClient webSocket;

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

const uint8_t LastTimeCount = 11;
uint32_t LastTime[LastTimeCount];
uint32_t LastCurrentTime = 0;
uint8_t  LastTimeID = 0;
bool LastTimeID_State = HIGH;
bool LastTimeID_LastState = HIGH;
uint32_t LastTimeID_State_ts = millis();

uint32_t TopTime = 599999;
uint count = 0;

int StartStopLastState;

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

void SetTime() {
  CurrentMinutes = (int)(CurrentTime / 60000) % 10;
  CurrenttSeconds = (int)(CurrentTime % 60000 / 10000);
  Currentseconds = (int)(CurrentTime % 60000 % 10000 / 1000);
  CurrentmSeconds = (int)(CurrentTime % 60000 % 1000 / 100);
  CurrentmiSeconds = (int)(CurrentTime % 60000 % 100 / 10);
  CurrentmilSeconds = (int)(CurrentTime % 60000 % 10);
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

void TimerLoop() {
  if (TimerState == HIGH && TimerLastState != TimerState) {
    
    for (int i = 9; i > 0; i--){
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

void printWifiState() {
  if (Connected == true) {
    lcd.setCursor(0,0);
    lcd.print("+");
  } else {
    lcd.setCursor(0,0);
    lcd.print("-");
  }
}

void printWsState() {
  if (ConnectedWS == true) {
    lcd.setCursor(2,0);
    lcd.print("+");
  } else {
    lcd.setCursor(2,0);
    lcd.print("-");
  }
}

void printSSID() {
  lcd.setCursor(0,1);
  lcd.print("Wifi:" + String(ssid[wifi_id]) + "     ");
}


void PrintTime() {
  lcd.setCursor(7,0);
  lcd.print(String(CurrentMinutes) + ":" + String(CurrenttSeconds) + String(Currentseconds) + "." + String(CurrentmSeconds) + String(CurrentmiSeconds) + String(CurrentmilSeconds) + "   ");
}

void TimePrintXY(uint32_t time, byte x, byte y, String name) {

  uint8_t minutes = (int)(time / 60000) % 10;
  uint8_t tSeconds = (int)(time % 60000 / 10000);
  uint8_t seconds = (int)(time % 60000 % 10000 / 1000);
  uint8_t mSeconds = (int)(time % 60000 % 1000 / 100);
  uint8_t miSeconds = (int)(time % 60000 % 100 / 10);
  uint8_t milSeconds = (int)(time % 60000 % 10);
  lcd.setCursor(x,y);
  lcd.print(name + String(minutes) + ":" + String(tSeconds) + String(seconds) + "." + String(mSeconds) + String(miSeconds) + String(milSeconds) + "    ");
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
    // For ESP8266
    //case WIFI_EVENT_STAMODE_CONNECTED:
      Serial.println("wifi connected");
      Connected = true;
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    // For ESP8266
    //case WIFI_EVENT_STAMODE_DISCONNECTED:
      Serial.println("wifi disconnected");
      Connected = false;
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    // For ESP8266
    //case WIFI_EVENT_STAMODE_GOT_IP:
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
    ResetTime();
    ssid_state_ts = millis();
    delay(250);
    connectToHost();
  }
  ssid_laststate = ssid_state;
}

void PrintCopyright(void) {
  lcd.setCursor(0,0);
  lcd.print("Made by VeZhD");
  lcd.setCursor(0,1);
  lcd.print("Code from Alekssaff");
  delay(1500);
}

void LastTimeIDChangeLoop() {
  LastTimeID_State = digitalRead(LAST_TIME_PIN);
  if (LastTimeID_State == LOW && LastTimeID_LastState == HIGH) {
    if (LastTimeID >= 0 && LastTimeID < LastTimeCount) {
      LastTimeID++;
    } else {
      LastTimeID = 0;
    }
    LastTimeID_State_ts = millis();
  }
  LastTimeID_LastState = LastTimeID_State;
}

void setup() {
  pinMode(SSID_PIN, INPUT_PULLUP);
  pinMode(LAST_TIME_PIN, INPUT_PULLUP);  // пин кнопки переключения предыдущего времени(1-10)
  Serial.begin(115200);

  // For ESP8266
  //EEPROM.begin(EEPROM_SIZE);
  
  // For ESP32/ESP32s2
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("failed to initialise EEPROM");
    delay(1000000);
  }
  if (EEPROM.read(0) >= 0 && EEPROM.read(0) < (sizeof(ssid) / sizeof(char *))) {
    wifi_id = EEPROM.read(0);
  }

  WiFi.onEvent(WiFiEvent);

  lcd.init(); // initialize the lcd 
  lcd.backlight();

  // event handler
  webSocket.onEvent(webSocketEvent);
  PrintCopyright();
  connectToHost();
  lcd.clear();
}

void loop() {
  TimerLoop();
  ssidChangeLoop();
  LastTimeIDChangeLoop();
  webSocket.loop();
  printWifiState();
  printWsState();
  PrintTime();
 if (LastTimeID_State_ts < millis() - 1500) {
    LastTimeID = LastTimeCount;
  }
  if (ssid_state_ts > millis() - 5000) {
    printSSID();
  }
  //else {
    if (LastTimeID == LastTimeCount) {
      TimePrintXY(TopTime, 0, 1, " Top: ");
    } 
    else {
      TimePrintXY(LastTime[LastTimeID], 0, 1, "Last: ");
    }
  }
  delay(1);
}
