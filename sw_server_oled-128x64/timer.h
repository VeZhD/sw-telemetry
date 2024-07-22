#ifndef DISPLAY_LASTTIME
  #define DISPLAY_LASTTIME 1
#endif
const uint8_t LastTimeCount = 100;
uint32_t LastTime[LastTimeCount];
uint32_t LastCurrentTime = 0;
uint8_t LastTimeID = 0;
bool LastTimeID_State = HIGH;
bool LastTimeID_LastState = HIGH;
uint32_t LastTimeID_State_ts = millis();

uint8_t timerState = 0;
uint32_t startTime = 0;
uint32_t currentTime = 0;
uint32_t lastChange;
String laptime = "";

uint32_t TopTime = 599999;
uint count = 0;

bool startStopState;
bool startStopLastState;

String startStopStateName;

void CalcTopTime() {
  if  ((24000 < currentTime) && (currentTime < TopTime) && (currentTime > uint32_t(TopTime / 1.15) or TopTime == 599999)) {
    TopTime = currentTime;
  }
  count++;
}


void ResetTime() {
  startTime = 0;
  currentTime = 0;
  startStopState = LOW;
  timerState = LOW;
  //TimerLastState = LOW;
  TopTime = 599999;
  
  for (int i = LastTimeCount - 1; i >= 0; i--) {
    LastTime[i] = 0;
  }

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

void ChangeModeLoop(void) {
  button01_State = digitalRead(button01);
  button03_State = digitalRead(button03);
  if (button01_State == LOW && button01_LastState != button01_State && button03_State == LOW) {
      if ( mode == "ss" ){
        mode = "lt";
        modeString = "lap-timer";
      } else {
        mode = "ss";
        modeString = "start-stop";
      }
    button01_LastState = button01_State;  
    }
  button03_LastState = button03_State;  
}