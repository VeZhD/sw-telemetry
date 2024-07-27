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

void InitRoutes(){

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
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
  
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest* request) {
    String resp;
    serializeJson(config, resp);
    request->send(200, "application/json", resp);
  });
  
  server.on("/config", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL, saveConfigFile);

  server.on("/favicon.ico",  HTTP_GET, [](AsyncWebServerRequest *request){    
    request->send_P(200, "image/png", favicon, favicon_png_len);
  });

  server.on("/NoSleep.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript", NoSleep_min_js_gz, NoSleep_min_js_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript", script_js_gz, script_js_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

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

}