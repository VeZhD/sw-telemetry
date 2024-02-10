bool ssid_state = HIGH;
bool ssid_laststate = HIGH;
uint32_t ssid_state_ts = millis();

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

const uint8_t LastTimeCount = 11;
uint32_t LastTime[LastTimeCount];
uint32_t LastCurrentTime = 0;
uint8_t LastTimeID = 0;
bool LastTimeID_State = HIGH;
bool LastTimeID_LastState = HIGH;
uint32_t LastTimeID_State_ts = millis();

uint32_t TopTime = 599999;
uint count = 0;

int StartStopLastState;
