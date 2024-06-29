#ifndef DISPLAY_LASTTIME
#define DISPLAY_LASTTIME 1
#endif

#define FONT_PIN      button01
#define LAST_TIME_PIN button02

bool startStopState;
String startStopStateName;
IPAddress myIP;
String apIP = "";
bool startStopLastState;
uint8_t timerState = 0;
uint32_t startTime = 0;
uint32_t currentTime = 0;
uint32_t lastChange;
String laptime = "";

bool  HotPlug_State;
bool  HotPlug_LastState = LOW;
bool  display_enable;

bool  button01_State;
bool  button01_LastState = HIGH;

bool  button02_State;
bool  button02_LastState = HIGH;

bool  button03_State;
bool  button03_LastState = HIGH;

uint8_t mode = 0;
String  modeString = "start-stop";

uint8_t wifi_id = 0;


// */

/* API is quite simple :
#include “esp_timer.h”

then call the function

int64_t esp_timer_get_time (void)

*/

bool ssid_state = HIGH;
bool ssid_laststate = HIGH;
uint32_t ssid_state_ts =  millis() ;

const uint8_t LastTimeCount = 100;
uint32_t LastTime[LastTimeCount];
uint32_t LastCurrentTime = 0;
uint8_t LastTimeID = 0;
bool LastTimeID_State = HIGH;
bool LastTimeID_LastState = HIGH;
uint32_t LastTimeID_State_ts = millis();

uint32_t TopTime = 599999;
uint count = 0;


void CalcTopTime() {
  if  ((1500 < currentTime) && (currentTime < TopTime) && (currentTime > uint32_t(TopTime / 1.15) or TopTime == 599999) && (ssid_state_ts < millis() - 7500)) {
    TopTime = currentTime;
  }
  count++;
}


void ResetTime() {
  startTime = 0;
  currentTime = 0;
  startStopState = 0;
  timerState = LOW;
  //TimerLastState = LOW;
  TopTime = 599999;
  
  for (int i = LastTimeCount - 1; i >= 0; i--) {
    LastTime[i] = 0;
  }

}


void connectToHost() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid_name[wifi_id], ssid_pass[wifi_id]);
}

void StartAPMode() {
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid_name[0], ssid_pass[0], 13);
  //WiFi.softAPConfig(apIP, apIPgate, IPAddress(255, 255, 255, 0));//для кастомного ip, работало почему-то не всегда 
}

void ssidChangeLoop() {
  //ssid_state = digitalRead(SSID_PIN);
  //if (ssid_state == LOW && ssid_laststate == HIGH) {
  button02_State = digitalRead(button02);
  button03_State = digitalRead(button03);
  if (button02_State == LOW && button02_LastState != button02_State && button03_State == LOW) {
    ResetTime();
    if (wifi_id < (sizeof(ssid_name) / sizeof(char *) - 1) && wifi_id >= 0) {
      wifi_id++;
      delay(250);
      connectToHost();
    } else {
      wifi_id = 0;
      delay(150);
      StartAPMode();
    }
    EEPROM.write(0, wifi_id);
    EEPROM.commit();
    ssid_state_ts = millis();
    button02_LastState = button01_State;  
    }
  button03_LastState = button03_State; 
  //ssid_laststate = ssid_state;

}


void LastTimeIDChangeLoop() {
  button02_State = digitalRead(button02);
  if (button02_State == LOW && button02_LastState == HIGH) {
    if ( LastTimeID >= 0 && LastTimeID < LastTimeCount - DISPLAY_LASTTIME && LastTime[LastTimeID + DISPLAY_LASTTIME] > 1 ) {
      LastTimeID++;
    } else {
      LastTimeID = 0;
    }
    LastTimeID_State_ts = millis();
  }
  
  button02_LastState = button02_State;
  
  if (LastTimeID_State_ts + 10500 < millis()) {
    LastTimeID = 0;
  }
}

void drawDigit(int ddX, int digit) {
  for (int y = 0; y < SizeY; y++) {
    for (int x = 0; x < SizeX; x++) {
      if (digits[Font_ID][digit][y][x] == 1) {
        display.drawPixel(x + ddX, y, SSD1306_WHITE);
      } else {
        display.drawPixel(x + ddX, y, SSD1306_BLACK);       
      }
    }
  }
}

void FontChangeLoop() {
  button01_State = digitalRead(button01);
  if (button01_State == LOW && button01_LastState == HIGH) {
    if (Font_ID >= 0 && Font_ID < Font_Count) {
      Font_ID++;
    } else {
      Font_ID = 0;
    }
    display.clearDisplay();
    EEPROM.write(3, Font_ID);
    EEPROM.commit();
  }
  button01_LastState = button01_State;
}


void TimePrintXY(uint32_t time, byte x, byte y, String name) {

  uint8_t minutes = (int)(time / 60000) % 10;
  uint8_t tSeconds = (int)(time % 60000 / 10000);
  uint8_t seconds = (int)(time % 60000 % 10000 / 1000);
  uint8_t mSeconds = (int)(time % 60000 % 1000 / 100);
  uint8_t miSeconds = (int)(time % 60000 % 100 / 10);
  uint8_t milSeconds = (int)(time % 60000 % 10);
  display.setCursor(x, y); 
  display.print(name + String(minutes) + ":" + String(tSeconds) + String(seconds) + "." + String(mSeconds) + String(miSeconds) + String(milSeconds));
}


void ChangeMode(void) {
  button01_State = digitalRead(button01);
  button03_State = digitalRead(button03);
  if (button01_State == LOW && button01_LastState != button01_State && button03_State == LOW) {
      if ( mode == 0 ){
        mode = 1;
        modeString = "lap-timer";
      } else {
        mode = 0;
        modeString = "start-stop";
      }
    button01_LastState = button01_State;  
    }
  button03_LastState = button03_State;  
}

void HotPlug_display(void) {
  HotPlug_State = digitalRead(HotPlug_pin);
  if (HotPlug_State == LOW && HotPlug_LastState != HotPlug_State) {
    delay(450);
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    display.clearDisplay();
    //display.display();
  }

  HotPlug_LastState = HotPlug_State;  
}

void TestPressButton01(void) {
   button01_State = digitalRead(button01);
  if (button01_State == LOW) {
  //if (button01_State == LOW && button01_LastState != button01_State) {
    display.setCursor(0, 40);
    display.print("1");
  } else {
    display.setCursor(0, 40);
    display.print("#");
  }

  //button01_LastState = button01_State;  
}

void TestPressButton02(void) {
   button02_State = digitalRead(button02);
  //if (button02_State == LOW && button02_LastState != button02_State) {
  if (button02_State == LOW) {
  display.setCursor(6, 40);
  display.print("2");
  } else {
  display.setCursor(6, 40);
  display.print("#");
  }

  //button02_LastState = button02_State;  
}

void TestPressButton03(void) {
   button03_State = digitalRead(button03);
  //if (button03_State == LOW && button03_LastState != button03_State) {
  if (button03_State == LOW) {
  display.setCursor(12, 40);
  display.print("3");
  } else {
  display.setCursor(12, 40);
  display.print("#");
  }

  //button03_LastState = button03_State;  
}