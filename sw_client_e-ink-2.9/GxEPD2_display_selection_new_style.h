// Display Library example for SPI e-paper panels from Dalian Good Display and boards from Waveshare.
// Requires HW SPI and Adafruit_GFX. Caution: the e-paper panels require 3.3V supply AND data lines!
//
// Display Library based on Demo Example from Good Display: https://www.good-display.com/companyfile/32/
//
// Author: Jean-Marc Zingg
//
// Version: see library.properties
//
// Library: https://github.com/ZinggJM/GxEPD2

// Supporting Arduino Forum Topics (closed, read only):
// Good Display ePaper for Arduino: https://forum.arduino.cc/t/good-display-epaper-for-arduino/419657
// Waveshare e-paper displays with SPI: https://forum.arduino.cc/t/waveshare-e-paper-displays-with-spi/467865
//
// Add new topics in https://forum.arduino.cc/c/using-arduino/displays/23 for new questions and issues

// NOTE: you may need to adapt or select for your wiring in the processor specific conditional compile sections below

// select the display class (only one), matching the kind of display panel
#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
//#define GxEPD2_DISPLAY_CLASS GxEPD2_3C

// select the display driver class (only one) for your  panel

//#define GxEPD2_DRIVER_CLASS GxEPD2_213     // GDE0213B1   122x250, IL3895, (HINK-E0213-G01), phased out
//#define GxEPD2_DRIVER_CLASS GxEPD2_213_B72 // GDEH0213B72 122x250, SSD1675A (IL3897), (HINK-E0213A22-A0 SLH1852)
//#define GxEPD2_DRIVER_CLASS GxEPD2_213_B73 // GDEH0213B73 122x250, SSD1675B, (HINK-E0213A22-A0 SLH1914)
#define GxEPD2_DRIVER_CLASS GxEPD2_213_B74 // GDEM0213B74 122x250, SSD1680, FPC-7528B)
//#define GxEPD2_DRIVER_CLASS GxEPD2_213_flex // GDEW0213I5F 104x212, UC8151 (IL0373), (WFT0213CZ16)
//#define GxEPD2_DRIVER_CLASS GxEPD2_213_M21 // GDEW0213M21 104x212, UC8151 (IL0373), (WFT0213CZ16)
//#define GxEPD2_DRIVER_CLASS GxEPD2_213_T5D // GDEW0213T5D 104x212, UC8151D, (WFT0213CZ16)
//#define GxEPD2_DRIVER_CLASS GxEPD2_213_BN // DEPG0213BN  122x250, SSD1680, (FPC-7528B), TTGO T5 V2.4.1, V2.3.1
//#define GxEPD2_DRIVER_CLASS GxEPD2_213_GDEY0213B74 // GDEY0213B74 122x250, SSD1680, (FPC-A002 20.04.08)

//#define GxEPD2_DRIVER_CLASS GxEPD2_290     // GDEH029A1   128x296, SSD1608 (IL3820), (E029A01-FPC-A1 SYX1553)
//#define GxEPD2_DRIVER_CLASS GxEPD2_290_T5  // GDEW029T5   128x296, UC8151 (IL0373), (WFT0290CZ10)
//#define GxEPD2_DRIVER_CLASS GxEPD2_290_T5D // GDEW029T5D  128x296, UC8151D, (WFT0290CZ10)
//#define GxEPD2_DRIVER_CLASS GxEPD2_290_I6FD // GDEW029I6FD  128x296, UC8151D, (WFT0290CZ10)
//#define GxEPD2_DRIVER_CLASS GxEPD2_290_T94 // GDEM029T94  128x296, SSD1680, (FPC-7519 rev.b)
//#define GxEPD2_DRIVER_CLASS GxEPD2_290_T94_V2 // GDEM029T94  128x296, SSD1680, (FPC-7519 rev.b), Waveshare 2.9" V2 variant
//#define GxEPD2_DRIVER_CLASS GxEPD2_290_BS // DEPG0290BS  128x296, SSD1680, (FPC-7519 rev.b)
//#define GxEPD2_DRIVER_CLASS GxEPD2_290_M06 // GDEW029M06  128x296, UC8151D, (WFT0290CZ10)
//#define GxEPD2_DRIVER_CLASS GxEPD2_290_GDEY029T94 // GDEY029T94 128x296, SSD1680, (FPC-A005 20.06.15)
//#define GxEPD2_DRIVER_CLASS GxEPD2_310_GDEQ031T10 // GDEQ031T10 240x320, UC8253, (no inking, backside mark KEGMO 3100)
//#define GxEPD2_DRIVER_CLASS GxEPD2_371     // GDEW0371W7  240x416, UC8171 (IL0324), (missing)
//#define GxEPD2_DRIVER_CLASS GxEPD2_370_TC1 // ED037TC1  280x480, SSD1677, (ICA-FU-20 ichia 2029), Waveshare 3.7"

// 3-color e-papers
//#define GxEPD2_DRIVER_CLASS GxEPD2_213c     // GDEW0213Z16 104x212, UC8151 (IL0373), (WFT0213CZ16)
//#define GxEPD2_DRIVER_CLASS GxEPD2_213_Z19c // GDEH0213Z19 104x212, UC8151D, (HINK-E0213A20-A2 2020-11-19)
//#define GxEPD2_DRIVER_CLASS GxEPD2_213_Z98c // GDEY0213Z98 122x250, SSD1680, (FPC-A002 20.04.08)

//#define GxEPD2_DRIVER_CLASS GxEPD2_290c     // GDEW029Z10  128x296, UC8151 (IL0373), (WFT0290CZ10)
//#define GxEPD2_DRIVER_CLASS GxEPD2_290_Z13c // GDEH029Z13  128x296, UC8151D, (HINK-E029A10-A3 20160809)
//#define GxEPD2_DRIVER_CLASS GxEPD2_290_C90c // GDEM029C90  128x296, SSD1680, (FPC-7519 rev.b)


#if defined(GxEPD2_DISPLAY_CLASS) && defined(GxEPD2_DRIVER_CLASS)

// somehow there should be an easier way to do this
#define GxEPD2_BW_IS_GxEPD2_BW true
#define GxEPD2_3C_IS_GxEPD2_3C true
#define IS_GxEPD(c, x) (c##x)
#define IS_GxEPD2_BW(x) IS_GxEPD(GxEPD2_BW_IS_, x)
#define IS_GxEPD2_3C(x) IS_GxEPD(GxEPD2_3C_IS_, x)

//#include "GxEPD2_selection_check.h"


#if defined(ESP32)
#define MAX_DISPLAY_BUFFER_SIZE 65536ul // e.g.
#if IS_GxEPD2_BW(GxEPD2_DISPLAY_CLASS)
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
#elif IS_GxEPD2_3C(GxEPD2_DISPLAY_CLASS) 
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8))
#endif
// adapt the constructor parameters to your wiring
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(CS_PIN, DC_PIN, RST_PIN,BUSY_PIN));
#endif

#endif
