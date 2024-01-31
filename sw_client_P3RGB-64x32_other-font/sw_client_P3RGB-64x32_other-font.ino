//https://espressif.github.io/arduino-esp32/package_esp32_index.json
/*
Add to ESP32-HUB75-MatrixPanel-I2S-DMA.h :

#define PIXEL_COLOUR_DEPTH_BITS 2
#define NO_GFX
#define USE_GFX_ROOT
*/

#include <WiFi.h>
#include <EEPROM.h>
#include <WebSocketsClient.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// Подключение Матрицы
#define R1_PIN 1
#define G1_PIN 3
#define B1_PIN 2
#define R2_PIN 4
#define G2_PIN 5
#define B2_PIN 6
#define E_PIN 7
#define A_PIN 8
#define B_PIN 9
#define C_PIN 10
#define D_PIN 11
#define CLK_PIN 13
#define LAT_PIN 12
#define OE_PIN 14

#define SSID_PIN        34  // пин кнопки переключения wifi сети
#define FONT_PIN        36  // пин кнопки переключения шрифта
#define LAST_TIME_PIN   38  // пин кнопки переключения предыдущего времени(1-10)
#define BRIGHTNESS_PIN  40  // пин кнопки яркости

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

// настройки размера матрицы
#define PANEL_RES_X 64   // Number of pixels wide of each INDIVIDUAL panel module.
#define PANEL_RES_Y 32   // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1    // Total number of panels chained one to another

byte      Brightness = 155;
bool      Brightness_State = HIGH;
bool      Brightness_LastState = HIGH;
uint32_t  Brightness_State_ts = 0;

HUB75_I2S_CFG::i2s_pins _pins = { R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN };

HUB75_I2S_CFG mxconfig(
  PANEL_RES_X,  // Module width
  PANEL_RES_Y,  // Module height
  PANEL_CHAIN,  // chain length
  _pins, HUB75_I2S_CFG::SHIFTREG, true);
//MatrixPanel_I2S_DMA dma_display;

MatrixPanel_I2S_DMA *dma_display = new MatrixPanel_I2S_DMA(mxconfig);
// end подключение матрицы

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
uint8_t  LastTimeID = 0;
bool LastTimeID_State = HIGH;
bool LastTimeID_LastState = HIGH;
uint32_t LastTimeID_State_ts = millis();

uint32_t TopTime = 599999;
uint count = 0;

uint16_t TIME_COLOR = dma_display->color444(0, 255, 0);
uint16_t DOT_COLOR = dma_display->color444(0, 255, 0);
const uint16_t BLACK_COLOR = dma_display->color444(0, 0, 0);

int StartStopLastState;

const uint8_t SizeX = 9;
const uint8_t SizeY = 17;
const uint8_t Font_Count = 2;
uint8_t Font_ID = 0;
bool Font_State = HIGH;
bool Font_LastState = HIGH;

const bool digits[Font_Count][10][SizeY][SizeX] = {{
  { // 0
    { 0,1,1,1,1,1,1,1,0 },
    { 1,0,1,1,1,1,1,0,1 },
    { 1,1,0,0,0,0,0,1,1 },

    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    
    { 1,0,0,0,0,0,0,0,1 },
    
    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    
    { 1,1,0,0,0,0,0,1,1 },
    { 1,0,1,1,1,1,1,0,1 },
    { 0,1,1,1,1,1,1,1,0 },
  },
  { // 1
    { 0,0,0,0,0,0,0,0,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },

    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    
    { 0,0,0,0,0,0,0,0,1 },
    
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,0,1 },
  },
  { // 2
    { 1,1,1,1,1,1,1,1,0 },
    { 0,1,1,1,1,1,1,0,1 },
    { 0,0,0,0,0,0,0,1,1 },

    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,1,1,1,1,1,0,1 },
    
    { 0,1,1,1,1,1,1,1,0 },
    
    { 1,0,1,1,1,1,1,0,0 },
    { 1,1,0,0,0,0,0,0,0 },
    { 1,1,0,0,0,0,0,0,0 },
    { 1,1,0,0,0,0,0,0,0 },
    { 1,1,0,0,0,0,0,0,0 },
    
    { 1,1,0,0,0,0,0,0,0 },
    { 1,0,1,1,1,1,1,1,0 },
    { 0,1,1,1,1,1,1,1,1 },
  },
  { // 3
    { 1,1,1,1,1,1,1,1,0 },
    { 0,1,1,1,1,1,1,0,1 },
    { 0,0,0,0,0,0,0,1,1 },

    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,1,1,1,1,1,0,1 },
    
    { 0,1,1,1,1,1,1,1,0 },
    
    { 0,0,1,1,1,1,1,0,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    
    { 0,0,0,0,0,0,0,1,1 },
    { 0,1,1,1,1,1,1,0,1 },
    { 1,1,1,1,1,1,1,1,0 },
  },
  { // 4
    { 1,0,0,0,0,0,0,0,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },

    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,0,1,1,1,1,1,0,1 },
    
    { 0,1,1,1,1,1,1,1,0 },
    
    { 0,0,1,1,1,1,1,0,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,0,1 },
  },
  { // 5
    { 0,1,1,1,1,1,1,1,1 },
    { 1,0,1,1,1,1,1,0,0 },
    { 1,1,0,0,0,0,0,0,0 },

    { 1,1,0,0,0,0,0,0,0 },
    { 1,1,0,0,0,0,0,0,0 },
    { 1,1,0,0,0,0,0,0,0 },
    { 1,1,0,0,0,0,0,0,0 },
    { 1,0,1,1,1,1,1,0,0 },
    
    { 0,1,1,1,1,1,1,1,0 },
    
    { 0,0,1,1,1,1,1,0,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    
    { 0,0,0,0,0,0,0,1,1 },
    { 0,1,1,1,1,1,1,0,1 },
    { 1,1,1,1,1,1,1,1,0 },
  },
  { // 6
    { 0,1,1,1,1,1,1,1,1 },
    { 1,0,1,1,1,1,1,1,0 },
    { 1,1,0,0,0,0,0,0,0 },

    { 1,1,0,0,0,0,0,0,0 },
    { 1,1,0,0,0,0,0,0,0 },
    { 1,1,0,0,0,0,0,0,0 },
    { 1,1,0,0,0,0,0,0,0 },
    { 1,0,1,1,1,1,1,0,0 },
    
    { 0,1,1,1,1,1,1,1,0 },
    
    { 1,0,1,1,1,1,1,0,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    
    { 1,1,0,0,0,0,0,1,1 },
    { 1,0,1,1,1,1,1,0,1 },
    { 0,1,1,1,1,1,1,1,0 },
  },
  { // 7
    { 1,1,1,1,1,1,1,1,0 },
    { 0,1,1,1,1,1,1,0,1 },
    { 0,0,0,0,0,0,0,1,1 },

    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    
    { 0,0,0,0,0,0,0,0,1 },
    
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
  
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,0,1 },
  },
  { // 8
    { 0,1,1,1,1,1,1,1,0 },
    { 1,0,1,1,1,1,1,0,1 },
    { 1,1,0,0,0,0,0,1,1 },

    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,0,1,1,1,1,1,0,1 },
    
    { 0,1,1,1,1,1,1,1,0 },
    
    { 1,0,1,1,1,1,1,0,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    
    { 1,1,0,0,0,0,0,1,1 },
    { 1,0,1,1,1,1,1,0,1 },
    { 0,1,1,1,1,1,1,1,0 },
  },
  { // 9
    { 0,1,1,1,1,1,1,1,0 },
    { 1,0,1,1,1,1,1,0,1 },
    { 1,1,0,0,0,0,0,1,1 },

    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,1,0,0,0,0,0,1,1 },
    { 1,0,1,1,1,1,1,0,1 },
    
    { 0,1,1,1,1,1,1,1,0 },
    
    { 0,0,1,1,1,1,1,0,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,1,1 },
    
    { 0,0,0,0,0,0,0,1,1 },
    { 0,1,1,1,1,1,1,0,1 },
    { 1,1,1,1,1,1,1,1,0 },
  }},
  {
  { // 0
    { 0, 0, 1, 1, 1, 1, 1, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 1, 1, 1 },
    { 1, 1, 0, 0, 0, 1, 1, 1, 1 },
    { 1, 1, 0, 0, 1, 1, 1, 1, 1 },
    { 1, 1, 0, 1, 1, 1, 0, 1, 1 },
    { 1, 1, 1, 1, 1, 0, 0, 1, 1 },
    { 1, 1, 1, 1, 0, 0, 0, 1, 1 },
    { 1, 1, 1, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 0, 1, 1, 1, 1, 1, 0, 0 },
  },
  { // 1
    { 0, 0, 0, 0, 0, 1, 1, 0, 0 },
    { 0, 0, 0, 0, 1, 1, 1, 0, 0 },
    { 0, 0, 0, 1, 1, 1, 1, 0, 0 },
    { 0, 0, 1, 1, 1, 1, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 1, 0, 0 },
    { 0, 0, 1, 1, 1, 1, 1, 1, 1 },
    { 0, 0, 1, 1, 1, 1, 1, 1, 1 },
  },
  { // 2
    { 0, 0, 1, 1, 1, 1, 1, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 1, 1, 1 },
    { 0, 0, 0, 0, 0, 1, 1, 1, 0 },
    { 0, 0, 0, 0, 1, 1, 1, 0, 0 },
    { 0, 0, 0, 1, 1, 1, 0, 0, 0 },
    { 0, 0, 1, 1, 1, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 0, 0, 0, 0, 0 },
    { 1, 1, 1, 0, 0, 0, 0, 0, 0 },
    { 1, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1 },
  },
  { // 3
    { 1, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 1, 1, 0, 0 },
    { 0, 0, 0, 0, 1, 1, 1, 0, 0 },
    { 0, 0, 0, 1, 1, 1, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 0, 1, 1, 1, 1, 1, 0, 0 },
  },
  { // 4
    { 0, 0, 0, 0, 0, 0, 0, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 1, 1, 1, 0 },
    { 0, 0, 0, 0, 1, 1, 1, 1, 0 },
    { 0, 0, 0, 1, 1, 1, 1, 1, 0 },
    { 0, 0, 1, 1, 1, 0, 1, 1, 0 },
    { 0, 1, 1, 1, 0, 0, 1, 1, 0 },
    { 1, 1, 1, 0, 0, 0, 1, 1, 0 },
    { 1, 1, 0, 0, 0, 0, 1, 1, 0 },
    { 1, 1, 0, 0, 0, 0, 1, 1, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 1, 1, 0 },
  },
  { // 5
    { 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 0, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 0, 1, 1, 1, 1, 1, 0, 0 },
  },
  { // 6
    { 0, 0, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 0,  },
    { 1, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 0 },
  },
  { // 7
    { 0, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 1, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 1, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 1, 0, 0 },
    { 0, 0, 0, 0, 1, 1, 0, 0, 0 },
    { 0, 0, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 0, 0 },
    { 0, 0, 0, 1, 1, 0, 0, 0, 0 },
    { 0, 0, 1, 1, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 1, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 0, 0, 0, 0, 0, 0 },
    { 1, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 1, 0, 0, 0, 0, 0, 0, 0 },
  },
  { // 8
    { 0, 0, 1, 1, 1, 1, 1, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 0, 1, 1, 1, 1, 1, 0, 0 },
  },
  { // 9
    { 0, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 0, 0, 1, 1, 1, 1, 1, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 1, 1, 1 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 0, 1, 1, 1, 1, 1, 0, 0 },
  }
}
};

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

void timerLoop() {
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

void updateDisplay() {
  dma_display->flipDMABuffer();  // Show the back buffer, set currently output buffer to the back (i.e. no longer being sent to LED panels)
  dma_display->clearScreen();    // Now clear the back-buffer
}

void printWifiState() {
  if (Connected == true) {
    dma_display->fillRect(int(PANEL_RES_X - 3), int(PANEL_RES_Y - 7), 3, 3, dma_display->color444(0, 255, 0));
  } else {
    dma_display->fillRect(int(PANEL_RES_X - 3), int(PANEL_RES_Y - 7), 3, 3, dma_display->color444(255, 0, 0));
  }
}

void printWsState() {
  if (ConnectedWS == true) {
    dma_display->fillRect(int(PANEL_RES_X - 3), int(PANEL_RES_Y - 3), 3, 3, dma_display->color444(0, 255, 0));
  } else {
    dma_display->fillRect(int(PANEL_RES_X - 3), int(PANEL_RES_Y - 3), 3, 3, dma_display->color444(255, 0, 0));
  }
}

void PrintSSID() {
  dma_display->setCursor(1, int(PANEL_RES_Y - 14));
  dma_display->setTextSize(1);  // size 1 == 8 pixels high
  dma_display->setTextColor(dma_display->color444(222, 222, 222));
  dma_display->print(String(ssid[wifi_id]));
}

void printBrightness() {
  dma_display->setTextColor(dma_display->color444(100, 100, 100));
  dma_display->setCursor(int(PANEL_RES_X - 6 * 3), int(PANEL_RES_Y - 14));  // start at top left, with 8 pixel of spacing
  dma_display->setTextSize(1);
  dma_display->print(Brightness);
}

void drawDigit(int ddX, int digit) {
  for (int y = 0; y < SizeY; y++) {
    for (int x = 0; x < SizeX; x++) {
      if (digits[Font_ID][digit][y][x] == 1) {
        dma_display->drawPixel(x + ddX, y, TIME_COLOR);
        /*
        }
      else {
        dma_display->drawPixel(x + ddX, y, BLACK_COLOR);
        */
      }
    }
  }
}

void PrintTime() {

  if (StartStopState == 0) {
    TIME_COLOR = dma_display->color444(0, 255, 0);
    DOT_COLOR = dma_display->color444(255, 0, 0);
  } else {
    TIME_COLOR = dma_display->color444(255, 0, 0);
    DOT_COLOR = dma_display->color444(0, 255, 0);
  }

  if (Font_ID == Font_Count) {
      const uint8_t font_size = 2;
    dma_display->setTextWrap(false);  // Don't wrap at end of line - will do ourselves
      dma_display->setCursor(1, 1);    // start at top left, with 8 pixel of spacing
    dma_display->setTextColor(TIME_COLOR);
    dma_display->setTextSize(font_size);  // size 1 == 8 pixels high
    dma_display->print(CurrentMinutes);
      dma_display->fillRect(12, 5, 2, 2, DOT_COLOR); // двоеточие 
      dma_display->fillRect(12, 9, 2, 2, DOT_COLOR); // двоеточие 
      dma_display->setCursor(15, 1);    // start at top left, with 8 pixel of spacing
    dma_display->print(CurrenttSeconds);
    dma_display->print(Currentseconds);
      dma_display->fillRect(38, 13, 2, 2, DOT_COLOR); // точка перед милисикундами
      dma_display->setCursor(41, 1);    // start at top left, with 8 pixel of spacing
    dma_display->print(CurrentmSeconds);
    dma_display->print(CurrentmiSeconds);
  } else {
    int dX = -1;
    // Minutes
    drawDigit(dX + 0, CurrentMinutes);
    // Seconds
    drawDigit(dX + (1 * SizeX) + 4, CurrenttSeconds);
    drawDigit(dX + (2 * SizeX) + 5, Currentseconds);
    // milliseconds
    drawDigit(dX + (3 * SizeX) + 9, CurrentmSeconds);
    drawDigit(dX + (4 * SizeX) + 10, CurrentmiSeconds);
    drawDigit(dX + (5 * SizeX) + 11, CurrentmilSeconds);
    // Двоеточие
    dma_display->fillRect(dX + (1 * SizeX) + 1, int(SizeY / 3), 2, 2, DOT_COLOR);
    dma_display->fillRect(dX + (1 * SizeX) + 1, int(SizeY - SizeY / 3), 2, 2, DOT_COLOR);
    // Точка
    dma_display->fillRect(dX + (3 * SizeX) + 6, SizeY - 2, 2, 2, DOT_COLOR);
  }
}

void PrintTimeSeconds() {
    const uint8_t font_size = 8;
    const uint8_t dy = 4;
    const uint8_t dot = (font_size/2+1);

    dma_display->setTextWrap(false);  // Don't wrap at end of line - will do ourselves
    dma_display->setCursor(0, dy);
    dma_display->setTextColor(TIME_COLOR);
    dma_display->setTextSize(font_size);  // size 1 == 8 pixels high
    dma_display->print(CurrentMinutes);
    dma_display->fillRect(5 * font_size + 1, 2 * font_size + 1, dot, dot, DOT_COLOR);  // двоеточие
    dma_display->fillRect(5 * font_size + 1, 4 * font_size + 2, dot, dot, DOT_COLOR);  // двоеточие
    dma_display->setCursor(5 * font_size + 7, dy);
    dma_display->print(CurrenttSeconds);
    dma_display->setCursor(10 * font_size + 8, dy);
    dma_display->print(Currentseconds);

}

void PrintTimeMilSeconds() {
    const uint8_t font_size = 8;
    const uint8_t dy = 4;
    const uint8_t dot = (font_size/2+1);

    dma_display->setTextWrap(false);  // Don't wrap at end of line - will do ourselves
    dma_display->setCursor(0, dy);
    dma_display->setTextColor(TIME_COLOR);
    dma_display->setTextSize(font_size);  // size 1 == 8 pixels high
    dma_display->fillRect(0, 6 * font_size + dot + dy/2, dot, dot, DOT_COLOR);  // точка перед милисикундами
    dma_display->setCursor(dot + 1, dy);
    dma_display->print(CurrentmSeconds);
    dma_display->setCursor(5 * font_size + dot + 2, dy);
    dma_display->print(CurrentmiSeconds);
    dma_display->setCursor(10 * font_size + dot + 3, dy);
    dma_display->print(CurrentmilSeconds);
}

void TimePrintXY(uint32_t time, byte x, byte y, String name) {

  uint8_t minutes = (int)(time / 60000) % 10;
  uint8_t tSeconds = (int)(time % 60000 / 10000);
  uint8_t seconds = (int)(time % 60000 % 10000 / 1000);
  uint8_t mSeconds = (int)(time % 60000 % 1000 / 100);
  uint8_t miSeconds = (int)(time % 60000 % 100 / 10);
  uint8_t milSeconds = (int)(time % 60000 % 10);

  dma_display->setCursor(x, y);  // 1 : 58
  dma_display->setTextWrap(false);
  dma_display->setTextColor(TIME_COLOR);
  dma_display->setTextSize(1);
  dma_display->print(name);
  dma_display->setTextColor(dma_display->color444(255, 0, 0));
  dma_display->drawPixel(x + name.length() * 6, y + 2, dma_display->color444(0, 0, 255));  // двоеточие
  dma_display->drawPixel(x + name.length() * 6, y + 4, dma_display->color444(0, 0, 255));  // двоеточие
  dma_display->setCursor(x + name.length() * 6 + 2, y);
  dma_display->print(minutes);
  dma_display->drawPixel(x + name.length() * 6 + 8, y + 2, dma_display->color444(0, 0, 255));  // двоеточие
  dma_display->drawPixel(x + name.length() * 6 + 8, y + 4, dma_display->color444(0, 0, 255));  // двоеточие
  dma_display->setCursor(x + name.length() * 6 + 10, y);
  dma_display->print(tSeconds);
  dma_display->print(seconds);
  dma_display->drawPixel(x + name.length() * 6 + 22, y + 6, dma_display->color444(0, 0, 255));  // точка перед милисикундами
  dma_display->setCursor(x + name.length() * 6 + 24, y);
  dma_display->print(mSeconds);
  dma_display->print(miSeconds);
  dma_display->print(milSeconds);
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

void FontChangeLoop() {
  Font_State = digitalRead(FONT_PIN);
  if (Font_State == LOW && Font_LastState == HIGH) {
    if (Font_ID >= 0 && Font_ID < Font_Count + 2) {
      Font_ID++;
    } else {
      Font_ID = 0;
    }
    EEPROM.write(3, Font_ID);
    EEPROM.commit();
  }
  Font_LastState = Font_State;
}

void LastTimeIDChangeLoop() {
  LastTimeID_State = digitalRead(LAST_TIME_PIN);
  if (LastTimeID_State == LOW && LastTimeID_LastState == HIGH) {
    if (LastTimeID >= 0 && LastTimeID < LastTimeCount - 1) {
      LastTimeID++;
    } else {
      LastTimeID = 0;
    }
    LastTimeID_State_ts = millis();
  }
  LastTimeID_LastState = LastTimeID_State;
}
    
void BrightnessChangeLoop() {
  Brightness_State = digitalRead(BRIGHTNESS_PIN);
  if (Brightness_State == LOW && Brightness_LastState == HIGH) {
    if (Brightness > 0 && Brightness < 255) {
      Brightness += 5;
    } else {
      Brightness = 125;
    }

    dma_display->setBrightness(Brightness);
    EEPROM.write(5, Brightness);
    EEPROM.commit();
    Brightness_State_ts = millis();
  }
  Brightness_LastState = Brightness_State;
}

void PrintCopyright(void) {
  dma_display->setTextColor(TIME_COLOR);
  dma_display->setCursor(8, 1);
  dma_display->print("Made by");
  dma_display->setCursor(14, 9);
  dma_display->print("VeZhD");
  dma_display->setCursor(5, 17);
  dma_display->print("Code from");
  dma_display->setCursor(5, 25);
  dma_display->print("Alekssaff");
  updateDisplay();
  delay(1500);
  dma_display->clearScreen();
}

void setup() {
  pinMode(SSID_PIN, INPUT_PULLUP);
  pinMode(FONT_PIN, INPUT_PULLUP);       // пин кнопки переключения шрифта
  pinMode(LAST_TIME_PIN, INPUT_PULLUP);  // пин кнопки переключения предыдущего времени(1-10)
  pinMode(BRIGHTNESS_PIN, INPUT_PULLUP); // пин кнопки яркости

  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("failed to initialise EEPROM");
    delay(1000000);
  }
  if ((EEPROM.read(3) >= 0) && (EEPROM.read(3) <= Font_Count)) {
    Font_ID = EEPROM.read(3);
  }
  if ((EEPROM.read(0) >= 0) && (EEPROM.read(0) < (sizeof(ssid) / sizeof(char *)))) {
    wifi_id = EEPROM.read(0);
  }
  if ((EEPROM.read(5) >= 0) && (EEPROM.read(5) <= 255)) {
    Brightness = EEPROM.read(5);
  }

  Serial.begin(115200);
  WiFi.onEvent(WiFiEvent);

  dma_display->begin();
  dma_display->clearScreen();
  dma_display->setBrightness(Brightness);  //0-255
  //dma_display->setLatBlanking(0);
  // event handler
  webSocket.onEvent(webSocketEvent);
  updateDisplay();
  PrintCopyright();
  updateDisplay();
  connectToHost();
}

void loop() {
  timerLoop();
  ssidChangeLoop();
  FontChangeLoop();
  LastTimeIDChangeLoop();
  BrightnessChangeLoop();
  webSocket.loop();

  printWifiState();
  printWsState();

switch (Font_ID) {
    case (Font_Count + 1):
      PrintTimeSeconds();
      break;
    case (Font_Count + 2):
      PrintTimeMilSeconds();
      break;
    default:
      PrintTime();
      if (LastTimeID_State_ts < millis() - 7500) {
        LastTimeID = 0;
      }
      TimePrintXY(LastTime[LastTimeID], 0, int(PANEL_RES_Y - 7), "LT" + String(LastTimeID));

      if (ssid_state_ts > millis() - 5500) {
        PrintSSID();
      } else {
        TimePrintXY(TopTime, 1, int(PANEL_RES_Y - 14), "Top");
      }
      if (Brightness_State_ts > millis() - 5000) {
        printBrightness();
      }
      else {
        TimePrintXY(TopTime, 1, int(PANEL_RES_Y - 14), "Top");
      }
  }
  updateDisplay();
  delay(1);
}
