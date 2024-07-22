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

}