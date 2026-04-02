#define JSON_CONFIG_FILE "/config_client.json"
// config vars
JsonDocument config;


String wifiMode;
JsonArray wifiList;
uint8_t wifi_id = 1;
//uint8_t Font_ID = 0;

void saveDefaultConfigFile()
// Save Config in JSON format
{
  // Uncomment if we need to format filesystem
  // SPIFFS.format();

  // Serial.println(F("Saving configuration..."));
  
  uint32_t chipId = 0;

  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    // chipId |= ((WiFi.macAddress().uint64_t() >> (40 - i)) & 0xff) << i;
  }

  // Create a JSON document
  JsonDocument json;
  json["timer"]["font_id"] = 0;
  json["timer"]["Brightness"] = 125;

  json["wifi"]["wifiid"] = 1;

  json["wifi"]["list"][0]["mode"] = "server";
  json["wifi"]["list"][0]["ssid"] = "SWc_" + String(chipId);
  json["wifi"]["list"][0]["pass"] = String(chipId) + "pass";
  json["wifi"]["list"][0]["ssid_channel"] = 1;
  json["wifi"]["list"][0]["ssid_hidden"] = 0;
  json["wifi"]["list"][0]["max_connection"] = 4;
  // json["wifi"]["list"][0]["wsSSL"] = "false";
  // json["wifi"]["list"][0]["wsHost"] = "192.168.4.1";
  // json["wifi"]["list"][0]["wsPath"] = "/";
  // json["wifi"]["list"][0]["wsPort"] = 80;

  // Open config file
  File configFile = SPIFFS.open(JSON_CONFIG_FILE, "w");
  // if (!configFile)
  // {
  //   // Error, file did not open
  //   Serial.println("failed to open config file for writing");
  // }
 
  // Serialize JSON data to write to file
  // serializeJsonPretty(json, Serial);
  serializeJson(json, configFile);
  // if (serializeJson(json, configFile) == 0)
  // {
  //   // Error writing file
  //   Serial.println(F("Failed to write to file"));
  // }
  // Close file
  configFile.close();
}

JsonDocument loadConfigFile()
// Load existing configuration file
{
  // Uncomment if we need to format filesystem
  // SPIFFS.format();
  JsonDocument json;
  // Read configuration from FS json
  // Serial.println("Mounting File System...");
 
  // May need to make it begin(true) first time you are using SPIFFS
  if (SPIFFS.begin(false) || SPIFFS.begin(true))
  {
    // Serial.println("mounted file system");
    if (SPIFFS.exists(JSON_CONFIG_FILE))
    {
      // The file exists, reading and loading
      // Serial.println("reading config file");
      File configFile = SPIFFS.open(JSON_CONFIG_FILE, "r");
      if (configFile)
      {
        // Serial.println("Opened configuration file");
        DeserializationError error = deserializeJson(json, configFile);
        // serializeJsonPretty(json, Serial);
        if (!error)
        {
          // Serial.println("Parsing JSON");
 
          // strcpy(testString, json["testString"]);
          // testNumber = json["testNumber"].as<int>();
 
          return json;
        }
        else
        {
          // Error loading JSON data
          // Serial.println("Failed to load json config");
          return json;
        }
      }
    }
    else {
      saveDefaultConfigFile();
      return loadConfigFile();
    }
  }
  // else
  // {
  //   // Error mounting file system
  //   Serial.println("Failed to mount FS");
  // }
  
  return json;
}

void InitConfig(){
  // if (SPIFFS.begin(false) || SPIFFS.begin(true)){
  //   // Serial.println("mounted file system");
  //   saveDefaultConfigFile();
  // }
  config = loadConfigFile();
  
  wifiList = config["wifi"]["list"].as<JsonArray>();
  wifi_id =  config["wifi"]["wifiid"].as<uint8_t>();
  wifiMode = config["wifi"]["list"][wifi_id]["mode"].as<String>();

  #ifdef FONT_PIN
  Font_ID = config["timer"]["font_id"].as<int>();
  #endif
}

void saveConfigFile(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
  File configFile = SPIFFS.open(JSON_CONFIG_FILE, "w");
  for (size_t i = 0; i < len; i++) {
    configFile.write(data[i]);
  }
  configFile.close();
  request->send(200, "application/json", String("1"));
  
  InitConfig();
  //ESP.restart();

}

void saveConfig(void){
    // Open config file
  File configFile = SPIFFS.open(JSON_CONFIG_FILE, "w");
  // if (!configFile)
  // {
  //   // Error, file did not open
  //   Serial.println("failed to open config file for writing");
  // }
  
  //Serial.println( config["wifi"]["wifiid"].as<uint8_t>() );

  // Serialize JSON data to write to file
  serializeJson(config, configFile);
  // serializeJsonPretty(config, Serial);
  // if (serializeJson(config, configFile) == 0)
  // {
  //   // Error writing file
  //   Serial.println(F("Failed to write to file"));
  // }
  // Close file
  configFile.close();

}
