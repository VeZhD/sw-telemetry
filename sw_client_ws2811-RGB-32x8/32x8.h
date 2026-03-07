const uint8_t SizeX = 4;
const uint8_t SizeY = 8;
// const uint8_t Font_Count = 1;

const bool digits[10][SizeY][SizeX] = {
  {
    { 0, 1, 1, 0 },
    { 1, 0, 0, 1 },
    { 1, 0, 0, 1 },
    { 1, 0, 0, 1 },
    { 1, 0, 0, 1 },
    { 1, 0, 0, 1 },
    { 1, 0, 0, 1 },
    { 0, 1, 1, 0 },
  },
  {
    { 0, 0, 1, 0 },
    { 0, 1, 1, 0 },
    { 0, 0, 1, 0 },
    { 0, 0, 1, 0 },
    { 0, 0, 1, 0 },
    { 0, 0, 1, 0 },
    { 0, 0, 1, 0 },
    { 0, 1, 1, 1 },
  },
  {
    { 0, 1, 1, 0 },
    { 1, 0, 0, 1 },
    { 0, 0, 0, 1 },
    { 0, 0, 0, 1 },
    { 0, 0, 1, 0 },
    { 0, 1, 0, 0 },
    { 1, 0, 0, 0 },
    { 1, 1, 1, 1 },
  },
  {
    { 0, 1, 1, 0 },
    { 1, 0, 0, 1 },
    { 0, 0, 0, 1 },
    { 0, 0, 1, 0 },
    { 0, 0, 0, 1 },
    { 0, 0, 0, 1 },
    { 1, 0, 0, 1 },
    { 0, 1, 1, 0 },
  },
  {
    { 1, 0, 0, 1 },
    { 1, 0, 0, 1 },
    { 1, 0, 0, 1 },
    { 0, 1, 1, 1 },
    { 0, 0, 0, 1 },
    { 0, 0, 0, 1 },
    { 0, 0, 0, 1 },
    { 0, 0, 0, 1 },
  },
  {
    { 1, 1, 1, 1 },
    { 1, 0, 0, 0 },
    { 1, 0, 0, 0 },
    { 1, 1, 1, 0 },
    { 0, 0, 0, 1 },
    { 0, 0, 0, 1 },
    { 1, 0, 0, 1 },
    { 0, 1, 1, 0 },
  },
  {
    { 0, 1, 1, 0 },
    { 1, 0, 0, 1 },
    { 1, 0, 0, 0 },
    { 1, 1, 1, 0 },
    { 1, 0, 0, 1 },
    { 1, 0, 0, 1 },
    { 1, 0, 0, 1 },
    { 0, 1, 1, 0 },
  },
  {
    { 1, 1, 1, 1 },
    { 0, 0, 0, 1 },
    { 0, 0, 0, 1 },
    { 0, 0, 1, 0 },
    { 0, 1, 0, 0 },
    { 0, 1, 0, 0 },
    { 0, 1, 0, 0 },
    { 0, 1, 0, 0 },
  },
  {
    { 0, 1, 1, 0 },
    { 1, 0, 0, 1 },
    { 1, 0, 0, 1 },
    { 0, 1, 1, 0 },
    { 1, 0, 0, 1 },
    { 1, 0, 0, 1 },
    { 1, 0, 0, 1 },
    { 0, 1, 1, 0 },
  },
  {
    { 0, 1, 1, 0 },
    { 1, 0, 0, 1 },
    { 1, 0, 0, 1 },
    { 0, 1, 1, 0 },
    { 0, 0, 0, 1 },
    { 0, 0, 0, 1 },
    { 1, 0, 0, 1 },
    { 0, 1, 1, 0 },
  }
};

const uint16_t colors[] = {matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255)};

uint16_t DotColor = matrix.Color(255, 0, 0);
uint16_t NumbersColor = matrix.Color(0, 255, 0);

const uint16_t blue = matrix.Color(0, 0, 255);
const uint16_t red = matrix.Color(255, 0, 0);
const uint16_t green = matrix.Color(0, 255, 0);

uint32_t PrintTicker_ts = millis();

byte Brightness = 25;
byte Brightness_change = 25;
bool Brightness_State = HIGH;
bool Brightness_LastState = HIGH;
uint32_t Brightness_State_ts = 0;

bool Brightness_less_State = HIGH;
bool Brightness_less_LastState = HIGH;
uint32_t Brightness_less_State_ts = millis();

void InitDisplay(){
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(Brightness);
  matrix.setTextColor(colors[0]);

}

void printWifiState() {
  if (Connected == true) {
    NumbersColor = matrix.Color(0, 255, 0); // Green
  } else {
    NumbersColor = matrix.Color(0, 0, 255); // Blue
  }
}

void printWsState() {
  if (ConnectedWS == true) {
    DotColor = matrix.Color(255, 0, 0); // Red
  } else {
    DotColor = matrix.Color(0, 0, 255); // Blue
  }
}

void drawDigit(int dX, int digit, uint16_t color) {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 4; x++) {
      if (digits[digit][y][x] == 1) {
        matrix.drawPixel(x + dX, y, color);
      }
    }
  }
}

void PrintTime() {

  if ( Font_ID == 0 ){
    int dX = 0;
    // minutes
    drawDigit(dX, CurrentMinutes, NumbersColor);
    // dots
    dX = 4;
    matrix.drawPixel(1 + dX, 3, DotColor);
    matrix.drawPixel(1 + dX, 5, DotColor);

    // tSeconds
    dX = 7;
    drawDigit(dX, CurrenttSeconds, NumbersColor);
    // seconds
    dX = 12;
    drawDigit(dX, Currentseconds, NumbersColor);
    // dot
    dX = 16;
    matrix.drawPixel(1 + dX, 7, DotColor);
    //drawDot(dX, NumbersColor);
    dX = 18;
    drawDigit(dX, CurrentmSeconds, NumbersColor);
    dX = 23;
    drawDigit(dX, CurrentmiSeconds, NumbersColor);
    dX = 28;
    drawDigit(dX, CurrentmilSeconds, NumbersColor);
  } else {
    matrix.setCursor(0, 0);
    matrix.setTextColor(NumbersColor);
    matrix.print(CurrentMinutes);
    matrix.setCursor(4, 0);
    matrix.setTextColor(DotColor);
    matrix.print(":");
    matrix.setCursor(8, 0);
    matrix.setTextColor(NumbersColor);
    matrix.print(String(CurrenttSeconds) + String(Currentseconds));
    matrix.setCursor(17, 0);
    matrix.setTextColor(DotColor);
    matrix.print(".");
    matrix.setCursor(21, 0);
    matrix.setTextColor(NumbersColor);
    matrix.print(String(CurrentmSeconds) + String(CurrentmiSeconds));
  }
}

/*
void TimePrintXY(uint32_t time, byte x, byte y, String name) {

  uint8_t minutes = (int)(time / 60000) % 10;
  uint8_t tSeconds = (int)(time % 60000 / 10000);
  uint8_t seconds = (int)(time % 60000 % 10000 / 1000);
  uint8_t mSeconds = (int)(time % 60000 % 1000 / 100);
  uint8_t miSeconds = (int)(time % 60000 % 100 / 10);
  uint8_t milSeconds = (int)(time % 60000 % 10);
  //myOLED.setFont(SmallFont);
  //myOLED.print("                      ", x, y);
  //myOLED.print(name + String(minutes) + ":" + String(tSeconds) + String(seconds) + "." + String(mSeconds) + String(miSeconds) + String(milSeconds) + "           ", x, y);
} */

void PrintTicker(String text,uint16_t colors, int l) {
  if ( PrintTicker_ts + 80 < millis() ) {
    PrintTicker_ts =  millis();
    int dx = -sizeof(char *);

    matrix.fillScreen(0);
    matrix.setCursor(shiftX, 0);
    matrix.setTextColor(colors);
    matrix.print(text + " " + text + " " + text + " " + text);
    //matrix.print(text);

    //if( --shiftX  + matrix.width() + 3 < dx * l ) {
    if( --shiftX  + matrix.width() < dx * l * 3 ) {
      shiftX = 0;
    }
    matrix.show();
  }

}

void BrightnessChangeLoop() {
  Brightness_State = digitalRead(BRIGHTNESS_PIN);
  Brightness_less_State = digitalRead(BRIGHTNESS_less_PIN);
  if (Brightness_State == LOW && Brightness_LastState == HIGH) {
  
    if ( Brightness >= Brightness_change && Brightness <= 250 - Brightness_change ) {
      Brightness += Brightness_change;
    } else {
      Brightness = Brightness_change;
    }

    if (Brightness > 251 || Brightness < Brightness_change ) {
      Brightness = 3 * Brightness_change;
    }
    
    matrix.setBrightness(Brightness);
    config["timer"]["Brightness"] = Brightness;
    saveConfig();

    Brightness_State_ts = millis() + 2500;
  
  } else if (Brightness_less_State == LOW && Brightness_less_LastState == HIGH) {
    
    if ( Brightness > Brightness_change && Brightness <= 255 ) {
      Brightness -= Brightness_change;
    } else {
      Brightness = 250;
    }
    
    if (Brightness > 251 || Brightness < Brightness_change - 5 ) {
      Brightness = 3 * Brightness_change;
    }

    matrix.setBrightness(Brightness);
    config["timer"]["Brightness"] = Brightness;
    saveConfig();
    
    Brightness_State_ts = millis() + 2500;
      
  }

  Brightness_LastState = Brightness_State;
  Brightness_less_LastState = Brightness_less_State;

}