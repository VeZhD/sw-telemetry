#define FONT_PIN      button01
#define LAST_TIME_PIN button02

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#include "font_data.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// const uint8_t DIGIT_WIDTH = 19;
// const uint8_t DIGIT_HEIGHT = 37;
const uint8_t SizeX = 19;
const uint8_t SizeY = 37;
const uint8_t Font_Count = 3;
bool Font_State = HIGH;
bool Font_LastState = HIGH;

bool  HotPlug_State;
bool  HotPlug_LastState = LOW;

void initDisplay() {
  delay(50);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.display();
}

bool hotPlug_display() {
  HotPlug_State = digitalRead(HotPlug_pin);
  if (HotPlug_State == LOW && HotPlug_LastState != HotPlug_State) {
    initDisplay();
  }
  HotPlug_LastState = HotPlug_State;  
  return !HotPlug_State;
}

void drawDigit(int x, int digit) {
  for (int row = 0; row < SizeY; row++) {
    uint32_t bits = (pgm_read_byte(&digits[Font_ID][digit][row][0]) << 16) |
                    (pgm_read_byte(&digits[Font_ID][digit][row][1]) << 8) |
                    pgm_read_byte(&digits[Font_ID][digit][row][2]);
    for (int col = 0; col < SizeX; col++) {
      if (bits & (1UL << (18 - col))) {
        display.drawPixel(x + col, row, SSD1306_WHITE);
      } else {
        display.drawPixel(x + col, row, SSD1306_BLACK);
      }
    }
  }
}

void printtime(long time) {
  int minutes = (int)(time / 60000) % 10;
  int tSeconds = (int)(time % 60000 / 10000);
  int seconds = (int)(time % 60000 % 10000 / 1000);
  int mSeconds = (int)(time % 60000 % 1000 / 100);
  int miSeconds = (int)(time % 60000 % 100 / 10);
  int milSeconds = (int)(time % 60000 % 10);

  display.clearDisplay();

  if (Font_ID == Font_Count) {

    display.setCursor(0, 0);  // Start at top-left corner

    display.setTextSize(3);  // Draw 3X-scale text
    display.setTextColor(SSD1306_WHITE);
    display.print(minutes);
    display.setTextSize(1);  // Draw 1X-scale text
    display.print(F(" "));
    display.fillRect(18, 6, 3, 3, SSD1306_WHITE);   // Draw :
    display.fillRect(18, 12, 3, 3, SSD1306_WHITE);  // Draw :
    display.setTextSize(3);                         // Draw 3X-scale text
    display.print(tSeconds);
    display.print(seconds);
    display.setTextSize(1);  // Draw 1X-scale text
    display.print(F(" "));
    display.fillRect(60, 18, 3, 3, SSD1306_WHITE);  // Draw .
    display.setTextSize(3);                         // Draw 3X-scale text
    display.print(mSeconds);
    display.print(miSeconds);
    display.println(milSeconds);
    display.setTextWrap(false);
    display.setTextSize(1);
    display.println("Wifi: " + config["wifi"]["list"][wifi_id]["ssid"].as<String>());
    // display.println("Wifi: " + WiFi.softAPSSID() ;
    if ( wifiMode == "server") {
      display.println("Pass: " + config["wifi"]["list"][wifi_id]["pass"].as<String>());
      // display.setCursor(117, 56);
      // display.print( WiFi.softAPgetStationNum() );
    } else {
      myIP = WiFi.localIP();
      //################
      apIP = String(myIP[0]) + ".";
      apIP += String(myIP[1]) + ".";
      apIP += String(myIP[2]) + ".";
      apIP += String(myIP[3]);
      //###################
      display.print("IPv4: " + apIP);
    }

  } else {
    int dX = 1;
    // Minutes
    drawDigit(dX + 0, minutes);
    // Seconds
    drawDigit(dX + (1 * SizeX) + 5, tSeconds);
    drawDigit(dX + (2 * SizeX) + 6, seconds);
    // milliseconds
    drawDigit(dX + (3 * SizeX) + 11, mSeconds);
    drawDigit(dX + (4 * SizeX) + 12, miSeconds);
    drawDigit(dX + (5 * SizeX) + 13, milSeconds);
    // Двоеточие
    display.fillRect(dX + (1 * SizeX) + 1, int(SizeY / 3), 3, 3, SSD1306_WHITE);
    display.fillRect(dX + (1 * SizeX) + 1, int(SizeY - SizeY / 3), 3, 3, SSD1306_WHITE);
    // Точка
    display.fillRect(dX + (3 * SizeX) + 7, SizeY - 3, 3, 3, SSD1306_WHITE);
  }

  display.setCursor(20, 40);
  display.print("mode: " + modeString);

  display.setCursor(0, 56);
  display.setTextColor(SSD1306_WHITE);
  if (digitalRead(SENSOR_PIN) == SensorLastState) {
    display.print("Sensor is working! ");
    startStopStateName = '1';
  } else {
    display.print("Sensor error!!! ");
    startStopStateName = '0';
  }

}

void printtime2(long time1, long time2) {
  int minutes1 = (int)(time1 / 60000) % 10;
  int tSeconds1 = (int)(time1 % 60000 / 10000);
  int seconds1 = (int)(time1 % 60000 % 10000 / 1000);
  int mSeconds1 = (int)(time1 % 60000 % 1000 / 100);
  int miSeconds1 = (int)(time1 % 60000 % 100 / 10);
  int milSeconds1 = (int)(time1 % 60000 % 10);

  int minutes2 = (int)(time2 / 60000) % 10;
  int tSeconds2 = (int)(time2 % 60000 / 10000);
  int seconds2 = (int)(time2 % 60000 % 10000 / 1000);
  int mSeconds2 = (int)(time2 % 60000 % 1000 / 100);
  int miSeconds2 = (int)(time2 % 60000 % 100 / 10);
  int milSeconds2 = (int)(time2 % 60000 % 10);

  display.clearDisplay();

  display.setCursor(0, 0);  // Start at top-left corner

  display.setTextSize(3);  // Draw 3X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.print(minutes1);
  display.setTextSize(1);  // Draw 1X-scale text
  display.print(F(" "));
  display.fillRect(18, 6, 3, 3, SSD1306_WHITE);   // Draw :
  display.fillRect(18, 12, 3, 3, SSD1306_WHITE);  // Draw :
  display.setTextSize(3);                         // Draw 3X-scale text
  display.print(tSeconds1);
  display.print(seconds1);
  display.setTextSize(1);  // Draw 1X-scale text
  display.print(F(" "));
  display.fillRect(60, 18, 3, 3, SSD1306_WHITE);  // Draw .
  display.setTextSize(3);                         // Draw 3X-scale text
  display.print(mSeconds1);
  display.print(miSeconds1);
  display.println(milSeconds1);

  display.setTextWrap(false);
  display.setTextSize(1);

  // display.println("mode:" + modeString);
  display.println(modeString);

  display.setCursor(0, 32);  // Start at top-left corner
  display.setTextSize(3);  // Draw 3X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.print(minutes2);
  display.setTextSize(1);  // Draw 1X-scale text
  display.print(F(" "));
  display.fillRect(18, 38, 3, 3, SSD1306_WHITE);   // Draw :
  display.fillRect(18, 44, 3, 3, SSD1306_WHITE);  // Draw :
  display.setTextSize(3);                         // Draw 3X-scale text
  display.print(tSeconds2);
  display.print(seconds2);
  display.setTextSize(1);  // Draw 1X-scale text
  display.print(F(" "));
  display.fillRect(60, 50, 3, 3, SSD1306_WHITE);  // Draw .
  display.setTextSize(3);                         // Draw 3X-scale text
  display.print(mSeconds2);
  display.print(miSeconds2);
  display.println(milSeconds2);

  // display.println("Wifi: " + config["wifi"]["list"][wifi_id]["ssid"].as<String>());
  // if ( wifiMode == "server") {
  //   display.println("Pass:" + config["wifi"]["list"][wifi_id]["pass"].as<String>());
  // } else {
  //   myIP = WiFi.localIP();
  //     //################
  //     apIP = String(myIP[0]) + ".";
  //     apIP += String(myIP[1]) + ".";
  //     apIP += String(myIP[2]) + ".";
  //     apIP += String(myIP[3]);
  //     //###################
  //     display.print("IPv4: " + apIP);
  //   }

  display.setCursor(20, 40);
  // display.print("mode: " + modeString);

  display.setCursor(0, 56);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  if (digitalRead(SENSOR_PIN) == SensorLastState) {
    display.print("Sensor is working! ");
    startStopStateName = '1';
  } else {
    display.print("Sensor error!!! ");
    startStopStateName = '0';
  }
  
}

void printip(void) {
  display.clearDisplay();
  display.setCursor(1, 32);  // Start at top-left corner
  display.setTextSize(1);    // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.println("Made by VeZhD");
  display.println("Code from Alekssaff");
  display.display();
  delay(1500);
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

    config["timer"]["font_id"] = Font_ID;
    saveConfig();

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
  display.print(time ? name + String(minutes) + ":" + String(tSeconds) + String(seconds) + "." + String(mSeconds) + String(miSeconds) + String(milSeconds) : name + "-:--.---");

}

void TestPressButton(void) {
  display.setCursor(0, 40);
  display.print(button01_State ? '#' : '1');
  display.print(button02_State ? '#' : '2');
  display.print(button03_State ? '#' : '3');

}
