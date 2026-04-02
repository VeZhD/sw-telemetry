void InitRoutes(){

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    // request->send_P(200, "text/html", index_html);
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", index_html_gz, index_html_gz_len);
    response->addHeader("Content-Type", "text/html");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/check", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, JSON_CONFIG_FILE );
  });

  server.on("/config", HTTP_GET, [](AsyncWebServerRequest* request) {
    String resp;
    serializeJson(config, resp);
    request->send(200, "application/json", resp);
  });
  
  server.on("/config", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL, saveConfigFile);

  server.on("/favicon.ico",  HTTP_GET, [](AsyncWebServerRequest *request){    
    request->send(200, "image/png", favicon, favicon_png_len);
  });

}