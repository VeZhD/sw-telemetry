
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

volatile bool  timerState = 0;
uint32_t startTime = 0;
uint32_t currentTime = 0;
uint32_t lastChange;

String laptime = "";

uint8_t  athleteNumber = 0;
const uint8_t numberOfAthlete = 2;

volatile bool timerStateArray[numberOfAthlete] = {0, 0};
uint32_t startTimeArray[numberOfAthlete] = {0, 0};
uint32_t currentTimeArray[numberOfAthlete] = {0, 0};

uint32_t TopTime = 599999;

// bool     startStopState;
// bool     startStopLastState;

String startStopStateName;

void CalcTopTime() {
  if  ((19000 < currentTime) && (currentTime < TopTime) && (currentTime > uint32_t(TopTime / 1.15) or TopTime == 599999)) {
    TopTime = currentTime;
  }

}

void ResetTime() {
  startTime = millis() + 9;
  currentTime = 0;
  timerState = 0;
  
  for (byte i = 0; i < numberOfAthlete; i++){
    timerStateArray[i] = 0;
    startTimeArray[i] = 0;
    currentTimeArray[i] = 0;
  }

  athleteNumber = 0;
  
  timerState = LOW;
  //TimerLastState = LOW;
  TopTime = 599999;
  
  for (byte i = 0; i < LastTimeCount; i++) {
    LastTime[i] = 0;
  }
  
  notifyClients();
}

uint ConvertTime(uint32_t time) {

  uint8_t minutes = (int)(time / 60000) % 10;
  uint8_t tSeconds = (int)(time % 60000 / 10000);
  uint8_t seconds = (int)(time % 60000 % 10000 / 1000);
  uint8_t mSeconds = (int)(time % 60000 % 1000 / 100);
  uint8_t miSeconds = (int)(time % 60000 % 100 / 10);
  uint8_t milSeconds = (int)(time % 60000 % 10);

  return minutes * 100000 + tSeconds * 10000 + seconds * 1000 + mSeconds * 100 + miSeconds * 10 + milSeconds;
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
      if ( mode == dlt ){
        mode = ss;
        modeString = "Start-Stop";
        ResetTime();
      } else if ( mode == ss ) {
        mode = lt;
        modeString = "Lap-timer";
        // ResetTime();
      } else if ( mode == lt ) {
        mode = dss;
        modeString = "Dual Start-Stop";
        ResetTime();
      } else if ( mode == dss ) {
        mode = sss;
        modeString = "Sequential Start-Stop";
        ResetTime();
      }
      else if ( mode == sss ) {
        mode = dlt;
        modeString = "Dual Lap-Timer";
        ResetTime();
      }
      else {
        mode = ss;
        modeString = "Start-Stop";
        ResetTime();
      }
    button01_LastState = button01_State;  
    }
  button03_LastState = button03_State;  
}

void IRAM_ATTR interruptTimer() { 
  // detachInterrupt(SENSOR_PIN);
  
  switch (mode) {
    case ss: {
      StartStop();
      break;
    }

    case lt: {
      LapTimer();
      break;
    }

    case dss: 
    case sss: {
      TwoAthletes(athleteNumber);
      break;
    }

    case dlt: {
      TwoAthletesLoop();
      break;
    }
  }
  
  // if ( ( sensorType == "npn" and gate == "no" ) or ( sensorType == "pnp" and gate == "nc" ) ) {
  //   attachInterrupt( SENSOR_PIN, interruptTimer, FALLING );
  // } else if ( ( sensorType == "pnp" and gate == "no" ) or ( sensorType == "npn" and gate == "nc" ) ) {
  //   attachInterrupt( digitalPinToInterrupt(SENSOR_PIN), interruptTimer, RISING );
  // }

}
