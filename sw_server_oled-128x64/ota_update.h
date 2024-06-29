//#include "esp32-hal.h"
//#include "WString.h"

const char header_html[] PROGMEM = "<!DOCTYPE html><html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8'/><meta name='viewport' content='width=device-width, initial-scale=1, minimum-scale=1.0, shrink-to-fit=no'><title>MG StopWatcher - Update!</title></head><body>";
const char footer_html[] PROGMEM = "</body></html>";
const char update_html[] PROGMEM = "<h1>Only .bin file</h1><form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update' required><input type='submit' value='Run Update'></form>";
const String FW_VERSION = "0.0.1Beta";


void OTAWeb_update_begin(){
  // HTTP basic authentication and Simple Firmware Update Form
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(web_user, web_pass))
      return request->requestAuthentication();
  //   request->send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
  	String view_html;
		view_html += header_html;
		view_html += "<h1>Actual Firmware Release: " + FW_VERSION + "</h1>";
		view_html += update_html;
		view_html += footer_html;
		request->send(200, "text/html", view_html);
   });

  // Simple Firmware Update Form
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
    //shouldReboot = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", Update.hasError()?"FAIL!!! Double-check steps and file \"*.bin\" and try again":"OK");
    response->addHeader("Connection", "close");
    request->send(response);
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index){
      //Serial.printf("Update Start: %s\n", filename.c_str());

#ifdef ESP8266
      Update.runAsync(true);
#endif

      if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)){
        Update.printError(Serial);
      }
    }
    if(!Update.hasError()){
      if(Update.write(data, len) != len){
        Update.printError(Serial);
      }
    }
    if(final){
 
      if (!Update.end(true)){
        Update.printError(Serial);
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "FAIL!!! Double-check steps and file \"*.bin\" and try again");
      } else {
        AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
        response->addHeader("Refresh", "20");  
        response->addHeader("Location", "/");
        request->send(response);
        //Serial.println("Update complete");
        //Serial.flush();
        delay(50);
        ESP.restart();
      }

    }
  });


  //server.on("/favicon.ico",  HTTP_GET, [](AsyncWebServerRequest *request){
  //   //request->send(200, "image/x-icon", favicon_ico);
    //request->send(200, "image/x-icon", "favicon");
    //response->addHeader("Content-Encoding", "gzip");
    //request->send(response);
    //});


//#ifdef ESP32
//  Update.onProgress(printProgress);
//#endif

}
