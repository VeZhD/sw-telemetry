#include "index.h"
#include "favicon.h"
#include "olddevices.h"

#if defined(OLD_index_html)
#include "old/html_get.h"
#include "old/script.js.gz.h"
#include "old/NoSleep.min.js.gz.h"
#endif

const char* PARAM_MESSAGE = "message";

const char changeTopTime[] PROGMEM = { R"rawliteral(
<html>
<head>
<title>Change TopTime</title>
</head>
<body>
<form action="get" method="get">
<input type="text" inputmode="numeric" name="message" pattern="[0-9]{4,6}" required>
<input type="submit" value="Change TopTime">
</form>
</body>
</html>
)rawliteral"};

const char changeTopTime1[] PROGMEM = "<html><head><title>Change TopTime</title></head><body><form action='get' method='get'><input type='text' inputmode='numeric' name='message' pattern='[0-9]{4,6}' list='defaultNumbers' required> <datalist id='defaultNumbers'>";
const char changeTopTime2[] PROGMEM = "</datalist><input type='submit' value='Change TopTime'></form></body></html>";

// User и паолль для обновелния, через web http://<host>/update, examle - http://192.168.4.1/update
// const char *web_user = "admin";
// const char *web_pass = "nimda";
const char header_html[] PROGMEM = "<!DOCTYPE html><html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8'/><meta name='viewport' content='width=device-width, initial-scale=1, minimum-scale=1.0, shrink-to-fit=no'><title>MG StopWatcher - Update!</title></head><body>";
const char footer_html[] PROGMEM = "</body></html>";
const char update_html[] PROGMEM = "<h1>Only .bin file</h1><form method='POST' action='/update' enctype='multipart/form-data'><input type='file' accept='.bin' name='update' required><input type='submit' value='Run Update'></form>";
const String FW_VERSION = "*.*.*";


void InitRoutes(){

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    // request->send_P(200, "text/html", new_html);
    // request->send(200, "text/html", new_html);

    // xxd -i <file>
    // Example: gzip -9 -k index.html && xxd -i index.html.gz | sed 's/ = / PROGMEM = /' | sed 's/unsigned/const unsigned/'
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", index_html_gz, index_html_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  
  server.on("/olddevices", HTTP_GET, [](AsyncWebServerRequest *request) {
    // request->send_P(200, "text/html", new_html);
    // request->send(200, "text/html", old_devices_html);
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", server_olddevices_html_gz, server_olddevices_html_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/check", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, JSON_CONFIG_FILE );
  });

  server.on("/laptime", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", laptime);
  });


  server.on("/toptime", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", String(TopTime));
  });
  
  server.on("/currentTime1", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", String(currentTimeArray[0]));
  });
  
  server.on("/currentTime2", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", String(currentTimeArray[1]));
  });

  server.on("/config", HTTP_GET, [](AsyncWebServerRequest* request) {
    String resp;
    serializeJson(config, resp);
    request->send(200, "application/json", resp);
  });

  server.on("/config", HTTP_POST, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "");
  }, NULL, saveConfigFile);

  server.on("/favicon.ico",  HTTP_GET, [](AsyncWebServerRequest *request){    
    // request->send_P(200, "image/png", favicon, favicon_png_len);
    request->send(200, "image/png", favicon, favicon_png_len);
  });


#if defined(OLD_index_html)
  server.on("/old", HTTP_GET, [](AsyncWebServerRequest *request) {
    // request->send_P(200, "text/html", index_html);
    request->send(200, "text/html", index_html);
  });

  server.on("/NoSleep.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    // AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript", NoSleep_min_js_gz, NoSleep_min_js_gz_len);
    AsyncWebServerResponse *response = request->beginResponse(200, "text/javascript", NoSleep_min_js_gz, NoSleep_min_js_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    // AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript", script_js_gz, script_js_gz_len);
    AsyncWebServerResponse *response = request->beginResponse(200, "text/javascript", script_js_gz, script_js_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
#endif

  server.on("/results", HTTP_GET, [](AsyncWebServerRequest* request) {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    for (int i = 0; i < LastTimeCount; i++) {
      arr.add(LastTime[i]);
    }
    String resp;
    serializeJson(doc, resp);
    request->send(200, "application/json", resp);
  });

  server.on("/changetoptime", HTTP_GET, [](AsyncWebServerRequest *request) {
    String changetoptime_index = changeTopTime1;

    for (int i = 0; i < LastTimeCount; i++) {
      if ( LastTime[i] > 1 ){
        changetoptime_index += "<option value='" + String(ConvertTime(LastTime[i])) + "'></option>";
        }
      }

    changetoptime_index += changeTopTime2;
    
    request->send(200, "text/html", changetoptime_index);
  });

    // Send a GET request to <IP>/get?message=<message>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String message;
    uint tmpt;
    uint tmpms;
    uint tmps;
    uint tmpm;
        
    if (request->hasParam(PARAM_MESSAGE)) {
      message = request->getParam(PARAM_MESSAGE)->value();

      tmpt = message.toInt();

      tmpms = (int)(tmpt % 1000);

      tmpt = (int)(tmpt / 1000);
      tmps = (int)(tmpt % 100);
      tmpm = (int)(tmpt / 100);
      TopTime = tmpms + tmps * 1000 + tmpm * 60000;          
    } else {
      message = "No message sent";
    }
      request->send(200, "text/plain", "Top Time: " + message + "\n\nMilliSeconds: " + String(tmpms)  + "\nSeconds: " + String(tmps)  + "\nMinutes: " + String(tmpm)  + "\n\nTopTime in MilliSeconds: " + String(TopTime) );
  });


 // HTTP basic authentication and Simple Firmware Update Form
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    // if(!request->authenticate(web_user, web_pass))
    //   return request->requestAuthentication();
  	String view_html;
		view_html += header_html;
		view_html += "<h1>Actual Firmware Release: " + FW_VERSION + "</h1>";
		view_html += update_html;
		view_html += footer_html;
		request->send(200, "text/html", view_html);
   });


  // Simple Firmware Update Form
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", Update.hasError()?"FAIL!!! Double-check steps and try again":"OK");
    response->addHeader("Connection", "close");
    request->send(response);
    },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index){
      //Serial.printf("Update Start: %s\n", filename.c_str());
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
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "FAIL!!! Double-check steps and try again");
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

}