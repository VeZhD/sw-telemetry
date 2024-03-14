#ifndef DISPLAY_LASTTIME
#define DISPLAY_LASTTIME 1
#endif

#ifndef SW_BASIC_OTA_HOSTNAME
#define SW_BASIC_OTA_HOSTNAME "SW_server"
#endif

#ifndef SW_BASIC_OTA_PASSWORD
#define SW_BASIC_OTA_PASSWORD "SW_serverPASSWORD"
#endif
// */

/* API is quite simple :
#include “esp_timer.h”

then call the function

int64_t esp_timer_get_time (void)

// Import required libraries for ESP8266
 /*
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <DNSServer.h>
// */


/*
void SW_Basic_OTA() {
  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname(SW_BASIC_OTA_HOSTNAME);
  // No authentication by default
  ArduinoOTA.setPassword(SW_BASIC_OTA_PASSWORD);
  ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";
    });

  ArduinoOTA.begin();
}
*/