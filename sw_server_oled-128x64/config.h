
#define JSON_CONFIG_FILE "/config.json"
// config vars
JsonDocument config;
String gate;
String sensorType;
String wifiMode;
JsonArray wifiList;
String mode;
String modeString = "start-stop";
uint StopDelay = 1650;    // задержка срабатывания на луч в миллисекундах
uint PrintDelay = 10000;  // задержка отображения результата в режиме кругового таймера
uint8_t wifi_id = 0;
uint8_t Font_ID = 0;
bool SensorState;
bool SensorLastState;

void saveDefaultConfigFile()
// Save Config in JSON format
{
  Serial.println(F("Saving configuration..."));
  
  // Create a JSON document
  JsonDocument json;
  json["timer"]["mode"] = "ss";
  json["timer"]["printDelay"] = 10000;
  json["timer"]["stopDelay"] = 1650;
  json["timer"]["font_id"] = 0;

  json["wifi"]["wifiid"] = 0;

  json["wifi"]["list"][0]["mode"] = "server";
  json["wifi"]["list"][0]["ssid"] = "StopWatcher";
  json["wifi"]["list"][0]["pass"] = "StopWatcher";

  json["sensor"]["type"] = "npn";
  json["sensor"]["gate"] = "nc";

  // Open config file
  File configFile = SPIFFS.open(JSON_CONFIG_FILE, "w");
  if (!configFile)
  {
    // Error, file did not open
    Serial.println("failed to open config file for writing");
  }
 
  // Serialize JSON data to write to file
  serializeJsonPretty(json, Serial);
  if (serializeJson(json, configFile) == 0)
  {
    // Error writing file
    Serial.println(F("Failed to write to file"));
  }
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
  Serial.println("Mounting File System...");
 
  // May need to make it begin(true) first time you are using SPIFFS
  if (SPIFFS.begin(false) || SPIFFS.begin(true))
  {
    Serial.println("mounted file system");
    if (SPIFFS.exists(JSON_CONFIG_FILE))
    {
      // The file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open(JSON_CONFIG_FILE, "r");
      if (configFile)
      {
        Serial.println("Opened configuration file");
        DeserializationError error = deserializeJson(json, configFile);
        serializeJsonPretty(json, Serial);
        if (!error)
        {
          Serial.println("Parsing JSON");
 
          // strcpy(testString, json["testString"]);
          // testNumber = json["testNumber"].as<int>();
 
          return json;
        }
        else
        {
          // Error loading JSON data
          Serial.println("Failed to load json config");
          return json;
        }
      }
    }
    else {
      saveDefaultConfigFile();
      return loadConfigFile();
    }
  }
  else
  {
    // Error mounting file system
    Serial.println("Failed to mount FS");
  }
  
  return json;
}

void InitConfig(){
  config = loadConfigFile();
  sensorType = config["sensor"]["type"].as<String>();
  gate = config["sensor"]["gate"].as<String>();
  wifiList = config["wifi"]["list"].as<JsonArray>();
  wifi_id =  config["wifi"]["wifiid"].as<uint8_t>();
  wifiMode = config["wifi"]["list"][wifi_id]["mode"].as<String>();
  mode = config["timer"]["mode"].as<String>();
  PrintDelay = config["timer"]["printDelay"].as<int>();
  StopDelay = config["timer"]["stopDelay"].as<int>();
  Font_ID = config["timer"]["font_id"].as<int>();

  if (sensorType == "npn"){
    pinMode(SENSOR_PIN, INPUT_PULLUP);  
  } else if (sensorType == "pnp") {
    pinMode(SENSOR_PIN, INPUT);    
  }

  if ( mode == "ss" ){
    modeString = "start-stop";
  } else {
    modeString = "lap-timer";
  }

  if ( ( sensorType == "npn" and gate == "no" ) or ( sensorType == "pnp" and gate == "nc" ) ) {
    // startStopState = !digitalRead(SENSOR_PIN);
    SensorState = LOW;
    SensorLastState = HIGH;
  } else if ( ( sensorType == "pnp" and gate == "no" ) or ( sensorType == "npn" and gate == "nc" ) ) {
    // startStopState = digitalRead(SENSOR_PIN);
    SensorState = HIGH;
    SensorLastState = LOW;
  }

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
  if (!configFile)
  {
    // Error, file did not open
    Serial.println("failed to open config file for writing");
  }
  
  Serial.println( config["wifi"]["wifiid"].as<uint8_t>() );

  // Serialize JSON data to write to file
  serializeJsonPretty(config, Serial);
  if (serializeJson(config, configFile) == 0)
  {
    // Error writing file
    Serial.println(F("Failed to write to file"));
  }
  // Close file
  configFile.close();

}
