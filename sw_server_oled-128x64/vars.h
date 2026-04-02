// pins for Lolin s2 mini, for other boards - check and replace the pins with yours:
#define SENSOR_PIN   6

#define button01    13
#define button02    12
#define button03    10
#define HotPlug_pin 11

// #if defined(ESP32)
//   #pragma message "ESP32 stuff happening!"
// #else
//   #error "This ain't a ESP32\ESPs2\ESPs3\etc., dumbo!"
// #endif

// pins for Lolin s3 mini and other boards on ESP32s3, check and replace the pins with yours:
#ifdef CONFIG_IDF_TARGET_ESP32S3
#undef SENSOR_PIN     // пин подключения датчика луча
#undef button01
#undef button02
#undef button03
#undef HotPlug_pin

#define SENSOR_PIN   6     // пин подключения датчика луча

#define button01     9
#define button02    10
#define button03     8
#define HotPlug_pin 11
#endif


bool  button01_State;
bool  button01_LastState = HIGH;
bool  button02_State;
bool  button02_LastState = HIGH;
bool  button03_State;
bool  button03_LastState = HIGH;


void notifyClients(void);

void ARDUINO_ISR_ATTR interruptTimer();

void StartStop();

void LapTimer();

void TwoAthletes(byte athlet);

void TwoAthletesLoop();