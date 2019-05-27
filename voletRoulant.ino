/*
   Gestion Volet Roulant
   https://www.ebay.fr/ 
   rechercher: ESP8266-Serial-WIFI-Witty-Cloud-Development-Board-ESP-12F-MINI-Nodemcu-CH340G

   ESP8266 Witty
   + Relais 2 channels avec optocoupleurs
   + Design 5A Range Current Sensor Module ACS712 Module Arduino Module
   + Optocoupler Isolation Voltage Test Board 8 Channel AC 220V

   V 0.2.0
   
   Author: Zzuutt
   https://github.com/zzuutt/gladys-esp8266-Witty---Shutter

*/
#include <FS.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
#include <WiFiManager.h>        //https://github.com/kentaylor/WiFiManager
#include <Shutters.h>           //https://github.com/marvinroger/arduino-shutters
#include <EEPROM.h>
#include <Ticker.h>
#include <OneButton.h>          //https://github.com/mathertel/OneButton
#include <Bounce2.h>            //https://github.com/thomasfredericks/Bounce2
#include "ESP8266TrueRandom.h"

String version_soft = "0.2.0";

class deviceVolet {
  public:
    unsigned int id, group, commandType;
    char deviceName[32];
    deviceVolet(unsigned int arg1, unsigned int arg2, char arg3[32], unsigned int arg4){
      id = arg1;
      group = arg2;
      strcpy(deviceName, arg3);
      commandType = arg4; // 0 = SWITCH; 1 = PUSH;
    }
};

class stateCourse {
  public:
    int pourcent;
    String info;
    stateCourse(int arg1, String arg2){
      pourcent = arg1;
      info = arg2;
    }
};

stateCourse measureCourse = {0, "Initialisation"};

deviceVolet volet = {0,0,"Volet 1",0};
String groupName[] = {"Aucun", "Salon", "Salle à manger", "Cuisine", "Chambre 1", "Chambre 2", "Chambre 3", "Couloir", "Buanderie", "Exterieur"};
int nbrGroup = sizeof(groupName) / sizeof(groupName[0]); //array size  ;

// WiFi Router Login - change these to your router settings
char gladys_server[40];
char gladys_port[6] = "8080";
char gladys_token[34] = "YOUR_GLADYS_TOKEN";
//default custom static IP
char static_ip[16] = "0.0.0.0";
char static_gw[16] = "192.168.0.254";
char static_sn[16] = "255.255.255.0";
char static_dns[16] = "192.168.0.254";
IPAddress _ip,_gw,_sn,_dns;

// Constants
const char* CONFIG_FILE_NETWORK = "/config_network.json";
const char* CONFIG_FILE = "/config_param.json";
const char* CONFIG_FILE_POSITION = "/last_position_level.json";

const byte eepromOffset = 0;
unsigned long upCourseTime = 30 * 1000;
unsigned long downCourseTime = 30 * 1000;
const float calibrationRatio = 0.1;
unsigned long temporaryTime;

int level = 0;
int motorStatus = 0;  // 0 = arret; 1 = monte; 2 = descend;
int lastInterStatus = 0;
int lastPositionLevel = 100;
int onRestartGoToPosition = 2; // 0 = Open; 1 = Close; 2 = LastPosition

//PIN du boutton poussoir reset
const int BUTTON_DEBUG_PIN = 4;
//Pin de la LED RGB couleur bleu
const int BLUE_PIN_LED = 13;
//Pin de la LED RGB couleur rouge
const int RED_PIN_LED = 15;
//Pin de la LED RGB couleur verte
const int GREEN_PIN_LED = 12;
//Pin Relais Descente
const int REL_DOWN_PIN = 0;
//Pin Relais Monté
const int REL_UP_PIN = 5;
//Pin INTER Descente
const int INTER_DOWN_PIN = 14;
//Pin INTER Monté
const int INTER_UP_PIN = 16;
//Pin du capteur de luminosite
const int LIGHT_PIN = A0;

bool ledStatus = false;
bool sensorCurrent = false;
bool sensorLight = true;
bool displayLight = true;
bool sensor = false;
int lightCurrentValue = 0;
int readManyTime = 1000;
unsigned long intervalCheck = 250;
unsigned long timeNow;
int tempoMoteur = 0;

bool courseTimeAuto = false;

bool debugMode = false;
bool espStart = false;
// Indicates whether ESP has WiFi credentials saved from previous session
bool initialConfig = false;
bool initialNetworkParam = false;
bool initialParam = false;

// UUIDs in binary form are 16 bytes long
byte uuidNumber[16]; 
String uuidStr;

Ticker blinker;
Bounce interUp = Bounce();
Bounce interDown = Bounce();

// Delay LongPress (in millisec)
int longPressDelay = 8000;  //8s
OneButton button(BUTTON_DEBUG_PIN, true);

// Set web server port number to 80
ESP8266WebServer server(80);

void red() {
  digitalWrite(RED_PIN_LED, HIGH);
  digitalWrite(GREEN_PIN_LED, LOW);
  digitalWrite(BLUE_PIN_LED, LOW);
}

void blue() {
  digitalWrite(RED_PIN_LED, LOW);
  digitalWrite(GREEN_PIN_LED, LOW);
  digitalWrite(BLUE_PIN_LED, HIGH);
}

void green() {
  digitalWrite(RED_PIN_LED, LOW);
  digitalWrite(GREEN_PIN_LED, HIGH);
  digitalWrite(BLUE_PIN_LED, LOW);
}

void white() {
  digitalWrite(RED_PIN_LED, HIGH);
  digitalWrite(GREEN_PIN_LED, HIGH);
  digitalWrite(BLUE_PIN_LED, HIGH);
}

void black() {
  digitalWrite(RED_PIN_LED, LOW);
  digitalWrite(GREEN_PIN_LED, LOW);
  digitalWrite(BLUE_PIN_LED, LOW);
}

void changeLedState(int pin) {
  byte red = bitRead(pin, 0);
  byte green = bitRead(pin, 1);
  byte blue = bitRead(pin, 2);

  if (red) {
    digitalWrite(RED_PIN_LED, !(digitalRead(RED_PIN_LED)));  //Invert Current State of LED  
  }
  if (green) {
    digitalWrite(GREEN_PIN_LED, !(digitalRead(GREEN_PIN_LED)));  //Invert Current State of LED
  }  
  if (blue) {   
    digitalWrite(BLUE_PIN_LED, !(digitalRead(BLUE_PIN_LED)));  //Invert Current State of LED
  }
}

void blinkLED(int c_red = 0, int c_green = 0, int c_blue = 0, float every = 0.5) {
  blinker.detach();
  black();
  //Initialize Ticker every 0.5s
  int pin = c_red + (c_green * 2) + (c_blue * 4);
  if (pin > 0){
    blinker.attach(every, changeLedState, pin);
  }
}

void debugState(){
  debugMode = !(debugMode);
  if(!debugMode){
    blinkLED(0,0,0,0.5);
  } else {
    blinkLED(0,1,0,0.1);
  }
}

void clearConfig() {
  // is configuration portal requested?
  Serial.println("Configuration portal requested");
  blinkLED(0, 0, 1, 0.2);
  WiFiManagerParameter custom_textIp("<p style=\"border-top: 1px black solid;padding: 5px;margin-top: 10px;\"><b>Network setting</b></p>");
  // IP 
  WiFiManagerParameter p_static_ip("static_ip", "IP static", static_ip, 16);
  WiFiManagerParameter p_static_gw("static_gw", "Gateway", static_gw, 16);
  WiFiManagerParameter p_static_sn("static_sn", "Subnet mask", static_sn, 16);
  WiFiManagerParameter p_static_dns("static_dns", "DNS", static_dns, 16);
  WiFiManagerParameter p_hintip("<small>*Hint: Leave the IP address empty or 0.0.0.0, if you want to use DHCP</small>");
  WiFiManagerParameter p_hint("<small>*Hint: if you want to reuse the currently active WiFi credentials, leave SSID and Password fields empty</small>");

  // Initialize WiFIManager
  WiFiManager wifiManager;
   
  //add all parameters here
  wifiManager.addParameter(&p_hint); 
  wifiManager.addParameter(&custom_textIp);
  wifiManager.addParameter(&p_static_ip);
  wifiManager.addParameter(&p_hintip);
  wifiManager.addParameter(&p_static_gw);
  wifiManager.addParameter(&p_static_sn);
  wifiManager.addParameter(&p_static_dns);
   
  // Sets timeout in seconds until configuration portal gets turned off.
  // If not specified device will remain in configuration mode until
  // switched off via webserver or device is restarted.
  // wifiManager.setConfigPortalTimeout(600);

  // It starts an access point
  // and goes into a blocking loop awaiting configuration.
  // Once the user leaves the portal with the exit button
  // processing will continue
  if (!wifiManager.startConfigPortal()) {
    Serial.println("Not connected to WiFi but continuing anyway.");
  } else {
    // If you get here you have connected to the WiFi
    Serial.println("Connected...yeey :)");
    blinkLED(0,0,1,1);

    // Getting posted form values and overriding local variables parameters
    // Config file is written regardless the connection state
    strcpy(static_ip, p_static_ip.getValue());
    strcpy(static_gw, p_static_gw.getValue());
    strcpy(static_sn, p_static_sn.getValue());
    strcpy(static_dns, p_static_dns.getValue());

    blinkLED(0, 0, 0, 0.5);
    black();
    // Writing JSON config file to flash for next boot
    Serial.println("Save data....");
    if (writeNetworkConfigFile()) {
      Serial.println("data saved :) !");
      blue();
      delay(1000);
    } else {
      red();
    }
  }
  Serial.println("Restart.....");
  delay(1000);
  ESP.restart();
  delay(5000);

}

void resetConfig(){
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  blinkLED(1,0,0,0.2);
  delay(3000);
  ESP.reset();  
}

bool readConfigNetworkFile() {
  // this opens the config file in read-mode
  File f = SPIFFS.open(CONFIG_FILE_NETWORK, "r");

  if (!f) {
    Serial.println("Configuration file not found");
    return false;
  } else {
    // we could open the file
    size_t size = f.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    // Read and store file contents in buf
    f.readBytes(buf.get(), size);
    // Closing file
    f.close();
    // Using dynamic JSON buffer which is not the recommended memory model, but anyway
    // See https://github.com/bblanchon/ArduinoJson/wiki/Memory%20model

    DynamicJsonBuffer jsonBuffer;
    // Parse JSON string
    JsonObject& json = jsonBuffer.parseObject(buf.get());
    // Test if parsing succeeds.
    if (!json.success()) {
      Serial.println("JSON parseObject() failed");
      return false;
    }
    json.printTo(Serial);

    // Parse all config file parameters, override
    // local config variables with parsed values
    if (json.containsKey("static_ip")) {
      strcpy(static_ip, json["static_ip"]);
    }

    if (json.containsKey("static_gw")) {
      strcpy(static_gw, json["static_gw"]);
    }

    if (json.containsKey("static_sn")) {
      strcpy(static_sn, json["static_sn"]);
    }
    
    if (json.containsKey("static_dns")) {
      strcpy(static_dns, json["static_dns"]);
    }
  }
  Serial.println("\nConfig file was successfully parsed");
  return true;
}

bool writeNetworkConfigFile() {
  Serial.println("Saving network config file");

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  // JSONify local configuration parameters
  json["static_ip"] = static_ip;
  json["static_gw"] = static_gw;
  json["static_sn"] = static_sn;
  json["static_dns"] = static_dns;

  // Open file for writing
  File f = SPIFFS.open(CONFIG_FILE_NETWORK, "w");
  if (!f) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  json.prettyPrintTo(Serial);
  // Write data to file and close it
  json.printTo(f);
  f.close();

  Serial.println("\nNetwork config file was successfully saved");
  return true;
}

bool readConfigFile() {
  Serial.println("Read configuration parameter file");
  // this opens the config file in read-mode
  File f = SPIFFS.open(CONFIG_FILE, "r");

  if (!f) {
    Serial.println("Configuration file not found");
    return false;
  } else {
    // we could open the file
    size_t size = f.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    // Read and store file contents in buf
    f.readBytes(buf.get(), size);
    // Closing file
    f.close();
    // Using dynamic JSON buffer which is not the recommended memory model, but anyway
    // See https://github.com/bblanchon/ArduinoJson/wiki/Memory%20model

    DynamicJsonBuffer jsonBuffer;
    // Parse JSON string
    JsonObject& json = jsonBuffer.parseObject(buf.get());
    // Test if parsing succeeds.
    if (!json.success()) {
      Serial.println("JSON parseObject() failed");
      return false;
    }
    json.printTo(Serial);

    // Parse all config file parameters, override
    // local config variables with parsed values     
    if (json.containsKey("gladys_server")) {
      strcpy(gladys_server, json["gladys_server"]);
    }

    if (json.containsKey("gladys_port")) {
      strcpy(gladys_port, json["gladys_port"]);
    }

    if (json.containsKey("gladys_token")) {
      strcpy(gladys_token, json["gladys_token"]);
    }

    if (json.containsKey("sensorLight")) {
      sensorLight = json["sensorLight"];
    }

    if (json.containsKey("sensorCurrent")) {
      sensorCurrent = json["sensorCurrent"];
    }

    if (json.containsKey("displayLight")) {
      displayLight = json["displayLight"];
    }

    if (json.containsKey("nbrGroup")) {
      nbrGroup =  json["nbrGroup"];

      for(int i=1; i < nbrGroup; i++){
        if (json.containsKey("Group_name" + String(i))) {
          groupName[i] = json["Group_name" + String(i)].as<String>();
          //strcpy(groupName[i], json["Group_name" + String(i)]);
        }        
      }
    }
           
    if (json.containsKey("ID_volet")) {
      volet.id = json["ID_volet"];
    }

    if (json.containsKey("NAME_volet")) {
      strcpy(volet.deviceName, json["NAME_volet"]);
    }

    if (json.containsKey("Group_volet")) {
      volet.group = json["Group_volet"];
    }

    if (json.containsKey("upCourseTime")) {
      upCourseTime = json["upCourseTime"];
    }

    if (json.containsKey("downCourseTime")) {
      downCourseTime = json["downCourseTime"];
    }

    if (json.containsKey("onRestartGoToPosition")) {
      onRestartGoToPosition = json["onRestartGoToPosition"];
    }

  }
  Serial.println("\nConfig file was successfully parsed");
  return true;
}

bool writeConfigFile() {
  Serial.println("Saving config file");

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  // JSONify local configuration parameters
   
  json["gladys_server"] = gladys_server;
  json["gladys_port"] = gladys_port;
  json["gladys_token"] = gladys_token;

  json["sensorLight"] = sensorLight;
  json["sensorCurrent"] = sensorCurrent;
  json["displayLight"] = displayLight;

  json["nbrGroup"] = nbrGroup;

  for(int i=1; i < nbrGroup; i++){
    json["Group_name" + String(i)] = groupName[i];
  }
  
  json["ID_volet"] = volet.id;

  json["NAME_volet"] = volet.deviceName;
 
  json["Group_volet"] = volet.group;

  json["upCourseTime"] = upCourseTime;

  json["downCourseTime"] = downCourseTime;

  json["onRestartGoToPosition"] = onRestartGoToPosition;

  // Open file for writing
  File f = SPIFFS.open(CONFIG_FILE, "w");
  if (!f) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  json.prettyPrintTo(Serial);
  // Write data to file and close it
  json.printTo(f);
  f.close();

  Serial.println("\nConfig file was successfully saved");
  return true;
}

void shuttersOperationHandler(Shutters* s, ShuttersOperation operation) {
  switch (operation) {
    case ShuttersOperation::UP:
      Serial.println("Shutters going up.");
      // TODO: Implement the code for the shutters to go up
      up();
      break;
    case ShuttersOperation::DOWN:
      Serial.println("Shutters going down.");
      // TODO: Implement the code for the shutters to go down
      down();
      break;
    case ShuttersOperation::HALT:
      Serial.println("Shutters halting.");
      // TODO: Implement the code for the shutters to halt
      halt();
      saveLastPostionLevel();
      break;
  }
}

void readInEeprom(char* dest, byte length) {
  for (byte i = 0; i < length; i++) {
    dest[i] = EEPROM.read(eepromOffset + i);
  }
}

void shuttersWriteStateHandler(Shutters* shutters, const char* state, byte length) {
  for (byte i = 0; i < length; i++) {
    EEPROM.write(eepromOffset + i, state[i]);
    #ifdef ESP8266
    EEPROM.commit();
    #endif
  }
}

void onShuttersLevelReached(Shutters* shutters, byte level) {
  Serial.print("Shutters at ");
  Serial.print(level);
  Serial.println("%");
}

Shutters shutters;

bool saveLastPostionLevel() {
  byte level = shutters.getCurrentLevel();
  Serial.println("last level : " + String(level) + "%");
  Serial.println("Saving position");

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  // JSONify local configuration parameters
  json["lastPositionLevel"] = level;

  // Open file for writing
  File f = SPIFFS.open(CONFIG_FILE_POSITION, "w");
  if (!f) {
    Serial.println("Failed to open last position level file for writing");
    return false;
  }

  json.prettyPrintTo(Serial);
  // Write data to file and close it
  json.printTo(f);
  f.close();

  Serial.println("\nLast position level was successfully saved");
  return true;
  
}

bool readLastPositionLevel() {
  // this opens the config file in read-mode
  File f = SPIFFS.open(CONFIG_FILE_POSITION, "r");

  if (!f) {
    Serial.println("Last position level file not found");
    return false;
  } else {
    // we could open the file
    size_t size = f.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    // Read and store file contents in buf
    f.readBytes(buf.get(), size);
    // Closing file
    f.close();
    // Using dynamic JSON buffer which is not the recommended memory model, but anyway
    // See https://github.com/bblanchon/ArduinoJson/wiki/Memory%20model

    DynamicJsonBuffer jsonBuffer;
    // Parse JSON string
    JsonObject& json = jsonBuffer.parseObject(buf.get());
    // Test if parsing succeeds.
    if (!json.success()) {
      Serial.println("JSON parseObject() failed");
      return false;
    }
    json.printTo(Serial);

    // Parse all config file parameters, override
    // local config variables with parsed values
    if (json.containsKey("lastPositionLevel")) {
      lastPositionLevel = json["lastPositionLevel"];
    }
  }
  Serial.println("\nLast position file was successfully parsed");
  return true;
}

void up() {
  motorStatus = 1;
  digitalWrite(REL_DOWN_PIN, 1);
  digitalWrite(REL_UP_PIN, 0);
}

void down() {
  motorStatus = 2;
  digitalWrite(REL_UP_PIN, 1); 
  digitalWrite(REL_DOWN_PIN, 0);
}

void halt() {
  motorStatus = 0;
  digitalWrite(REL_UP_PIN, 1); 
  digitalWrite(REL_DOWN_PIN, 1);
  if(gladys_server != "" && strcmp(gladys_server,"0.0.0.0") != 0) {
    sendStateToGladys(shutters.getCurrentLevel()); 
  }
}

void displayData(){
  if(!debugMode){
    displayLight = !(displayLight);
    Serial.println("\ndisplayLight :" + String(displayLight));  
  } else {
    Serial.println("\n............DATAS..........\n");
    Serial.print("Local ip: ");
    Serial.println(WiFi.localIP());
    Serial.print("Subnet mask: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("\nGladys server: \n   IP: ");
    Serial.println(gladys_server);
    Serial.print("   Port server: ");
    Serial.println(gladys_port);
    Serial.print("   Token: ");
    Serial.println(gladys_token);
    Serial.print("\nSensor Light is used: ");
    if(sensorLight){
      Serial.print("true");
      Serial.println("  Value: " + String(lightCurrentValue));
    } else {
      Serial.println("false");
    }
    Serial.print("Sensor Current is used: ");
    if(sensorCurrent){
      Serial.println("true");
    } else {
      Serial.println("false");
    }    
    Serial.print("\nPosition of the shutter after power failure: ");
    //onRestartGoToPosition = 2; // 0 = Open; 1 = Close; 2 = LastPosition
    if(onRestartGoToPosition == 0) Serial.println("Open");
    if(onRestartGoToPosition == 1) Serial.println("Close");
    if(onRestartGoToPosition == 2) Serial.println("LastPosition");
    Serial.print("\nDevice: \n");
    Serial.println("   -ID: " + String(volet.id) + "  -Name: " + String(volet.deviceName) + "  -group: " + groupName[volet.group]);
    Serial.println("\nUp course time: " + String(upCourseTime) + " ms");
    Serial.println("Down course time: " + String(downCourseTime) + " ms");
    Serial.println("\n............DATAS..........\n");    
  }
}

bool sensorStateLCR(){
  unsigned long timeCheck;
  timeNow = millis();
  timeCheck = timeNow + intervalCheck;
  while(timeNow < timeCheck){
    timeNow = millis();
    yield();  
  }
  if(sensorLight){
    lightCurrentValue = analogRead(LIGHT_PIN);
    if(lightCurrentValue > 300){
      return true;
    } else {
      return false;
    }
  } else {
    if(displayLight){
      return true;
    } else {
      return false;
    }
  }
}

int sensorStateCurrent(){
  unsigned long timeCheck;
  timeNow = millis();
  timeCheck = timeNow + intervalCheck;
  while(timeNow < timeCheck){
    timeNow = millis();  
  }  
  if(sensorCurrent && tempoMoteur >= 10){
      lightCurrentValue = analogRead(LIGHT_PIN);
      tempoMoteur = 11;  
  } else {
    tempoMoteur++;
    //Serial.println("tempoMoteur" + String(tempoMoteur));
  }
  server.handleClient();
  yield();
  return lightCurrentValue;
}

void interUD(){
  interUp.update();
  interDown.update();
  //int U = !(interUp.read());
  //int D = !(interDown.read());

  int interStatus = !(interUp.read())*10 + !(interDown.read());
  if(interStatus != lastInterStatus) {
    lastInterStatus = interStatus;
    switch (interStatus) {
      //monte
      case 10:
      if(motorStatus != 1){
        shutters.setLevel(0);
        Serial.println("Going to level 0%"); 
      }
      break;
  
      //descend
      case 1:
      if(motorStatus != 2){
        shutters.setLevel(100);
        Serial.println("Going to level 100%");  
      }
      break;
  
      //stop
      case 11:
      case 0:
      if(motorStatus != 0){
        shutters.stop();  
      }
      break;
    }
  }
}

void getHeatindex() {
  bool commandState = false;
  String cmd;
  int vposition = 0;
  if(debugMode){
    Serial.println("-------- DEBUG --------");
  }
  bool tokenOk = determinateToken();

  if (tokenOk){
    bool readArgId = determinateID();
    if(readArgId){
      if(server.hasArg("cmd")){
        cmd = server.arg("cmd");
        if(debugMode){
          Serial.print("Order received - command: " + cmd);
        }
        if(cmd == "open"){
          shutters.setLevel(0); // Go to 0%
          commandState = true;      
        }
        if(cmd == "close"){
          shutters.setLevel(100); // Go to 100%
          commandState = true;  
        }
        if(cmd == "stop"){
          shutters.stop(); // stop
          commandState = true;  
        }
        if(cmd == "goto"){
          if (server.hasArg("position")) { //just do the checks if the parameter is available
            vposition = server.arg("position").toInt();
            if(vposition >= 0 && vposition <= 100){
              shutters.setLevel(vposition); 
              commandState = true; 
            }
            if(debugMode){
              Serial.print("  Goto: " + String(vposition) + "%");
            }            
          }          
        }
        if(debugMode){
          Serial.println("");
        }
      }    
    }
  }
  if(debugMode){
    Serial.println("-------- DEBUG --------");
  }

  if(commandState){
    server.send(200, "text/plain", "{\"OK\":\"OK\"}" );
  } else {
    server.send(404, "text/plain", "404: Not found");
  }
}

bool determinateToken() {
  bool readInf = false;
  if (server.hasArg("token")) { //just do the checks if the parameter is available
      if ( String(gladys_token) == server.arg("token")){
          readInf = true;
      } else if(debugMode){
        Serial.println("ERROR Token");
        Serial.println("Token received: " + server.arg("token"));
        Serial.println("Token saved   : " + String(gladys_token));
      }
  }
  return readInf;
}

bool determinateID() {
  bool findDevice = false;
  if (server.hasArg("deviceid")) { //just do the checks if the parameter is available
      int readInf = server.arg("deviceid").toInt();
      if(debugMode){
        Serial.println("Device concerned ID: " + String(readInf));
      }
      if(readInf == volet.id){
        findDevice = true;
      }
  }
  return findDevice;
}

void sendConfig(){
  bool stateCommand = false;
  if(server.hasArg("mode")) {
    if(server.arg("mode") == "direct") {
      stateCommand = true;       
    }
  }
  if(debugMode || stateCommand){
    Serial.println("Generate config file");
    // Generate a new UUID
    ESP8266TrueRandom.uuid(uuidNumber);
    uuidStr = ESP8266TrueRandom.uuidToString(uuidNumber);
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();

    json["version"] = version_soft;
    if(!stateCommand && debugMode) {
      //json["sensorLight"] = sensorLight;
      //json["sensorCurrent"] = sensorCurrent;
      if(sensorLight) {
        json["sensor"] = "1";
      }
      if(sensorCurrent) {
        json["sensor"] = "2";
      }
      json["displayLight"] = displayLight;
        
      json["gladys_server"] = gladys_server;
      json["gladys_port"] = gladys_port;
      json["gladys_token"] = gladys_token;
      json["nbrGroup"] = nbrGroup; 
      for(int i = 1; i < nbrGroup; i++){
        json["groupName" + String(i)] = groupName[i];
      }
    }
    
    if(stateCommand) {
      json["t"] = gladys_token; 
    }
     
    json["voletId"] = volet.id; 
 
    json["voletName"] = volet.deviceName; 
  
    json["voletGroup"] = groupName[volet.group];  
  
    //json["voletType"] = volet.type;

    if(!stateCommand && debugMode) {
      json["voletGroup"] = volet.group;
      json["onRestartGoToPosition"] = onRestartGoToPosition; 
      json["secretKey"] = uuidStr;
    }
    
    String config_json;
    json.prettyPrintTo(config_json);
    sendHeaderAccess();
    server.send(200, "text/json", config_json);
        
  } else {
    sendHeaderAccess();
    server.send(404, "text/json", "{\"error\":\"Access denied\"}");
  }
  
}

void sendHeaderAccess() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Max-Age", "10000");
    server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");  
}

void saveConfig(){
  bool configOk = false;
  Serial.println("Receive data");
  if(debugMode && server.args() > 0){
    if(server.hasArg("secretKey")) {
      if(server.arg("secretKey") == uuidStr) {
        Serial.println("secret key is ok");
        String temp;
        if (server.hasArg("gladys_server")) {
          temp = server.arg("gladys_server");
          temp.toCharArray(gladys_server, temp.length()+1); 
        }
    
        if (server.hasArg("gladys_port")) {
          temp = server.arg("gladys_port"); 
          temp.toCharArray(gladys_port, temp.length()+1); 
        }
    
        if (server.hasArg("gladys_token")) {
          temp = server.arg("gladys_token"); 
          temp.toCharArray(gladys_token, temp.length()+1);
        }
    
//        if (server.hasArg("invertInputDataPortB")) {
//          invertInputDataPortB = server.arg("invertInputDataPortB");
//        } else {
//          invertInputDataPortB = false;
//        }
    
        if (server.hasArg("sensor")) {
          sensorLight = true;
          sensorCurrent = false;
          if(server.arg("sensor") == "2") {
            sensorLight = false;
            sensorCurrent = true;
          }
        }
        
        if (server.hasArg("displayLight")) {
          displayLight = server.arg("displayLight");
        } else {
          displayLight = false;
        }

        if (server.hasArg("nbrGroup")) {
          for(int i = 1; i < server.arg("nbrGroup").toInt(); i++){
            if (server.hasArg("groupName" + String(i))) {
              groupName[i] = server.arg("groupName" + String(i));
            }
          }
          for(int i = 1; i < server.arg("nbrGroup").toInt(); i++){
              Serial.println("gName array "+ String(i) + ":" + groupName[i]);
          }
        }
        
        if (server.hasArg("voletId")) {
          volet.id = server.arg("voletId").toInt();
        }
    
        if (server.hasArg("voletName")) {
          temp = server.arg("voletName"); 
          temp.toCharArray(volet.deviceName, temp.length()+1);
        }
    
        if (server.hasArg("voletGroup")) {
          volet.group = server.arg("voletGroup").toInt();
        }
    
        if (server.hasArg("voletType")) {
          volet.commandType = server.arg("voletType").toInt();
        }

        if (server.hasArg("onRestartGoToPosition")) {
          onRestartGoToPosition = server.arg("onRestartGoToPosition").toInt();
        }

        writeConfigFile();
        configOk = true;
        initialParam = false;
        blinkLED(0,0,0,1);
        blue();
      }
    }
  }
  sendHeaderAccess();
  if (configOk){  
    server.send(200, "text/json", "{\"state\":\"ok\"}");
  } else {
    server.send(404, "text/json", "{\"error\":\"Access denied\"}");
  }
}

void settingCourseTime(){
  if(debugMode){
    if (server.hasArg("cmd")){
      String cmd = server.arg("cmd");
      if(cmd == "posUP"){
        up();
        Serial.println("Position volet : Monte");
      }
      if(cmd == "posDOWN"){
        down();
        Serial.println("Position volet : Descend");
      }
      if(cmd == "posSTOP"){
        halt();
        Serial.println("Position volet : Stop");
      }
      if(cmd == "startUP"){
        up();
        temporaryTime = millis();
        Serial.println("Mesure temps de monté volet : Monte"); 
      }
      if(cmd == "stopUP"){
        upCourseTime = millis()-temporaryTime;
        halt();
        Serial.println("Mesure temps de monté volet : Stop");  
        Serial.print("durée montée :");
        Serial.println(upCourseTime);
        shutters.setCourseTime(upCourseTime, downCourseTime);
        writeConfigFile();
      }
      if(cmd == "startDOWN"){
        down();
        temporaryTime = millis();
        Serial.println("Mesure temps de descente volet : Descend");  
      }
      if(cmd == "stopDOWN"){
        downCourseTime = millis()-temporaryTime;
        halt();
        Serial.println("Mesure temps de descente volet : Stop");  
        Serial.print("durée descente :");
        Serial.println(downCourseTime);
        shutters.setCourseTime(upCourseTime, downCourseTime);
        writeConfigFile();
      }
      sendHeaderAccess();
      server.send(200, "text/json", "{\"ok\":\"ok\"}");
    }
  } else {
    sendHeaderAccess();
    server.send(404, "text/json", "{\"error\":\"Access denied\"}"); 
  }
}

void settingAutoCourseTime(){
  if(debugMode){
    if (server.hasArg("cmd")){
      String cmd = server.arg("cmd");
      if(cmd == "startAuto"){
        sendHeaderAccess();
        server.send(200, "text/json", "{\"ok\":\"ok\"}");
        courseTimeAuto = true;
        Serial.println("Mesure temps de monté / descente volet Automatique");
        //stop volet
        halt();
        measureCourse.pourcent = 20;
        measureCourse.info = "Mesure du courant au repos";
        server.handleClient();    
        //lecture courant au repos
        tempoMoteur = 100;
        int valueCurrentMotorStandby = sensorStateCurrent();
        Serial.println("Valeur courant standby :" + String(valueCurrentMotorStandby));
        lightCurrentValue = valueCurrentMotorStandby + 500;
        int valueCurrentMotor = lightCurrentValue;
        measureCourse.pourcent = 40;
        measureCourse.info = "Je ferme le volet<br />Ce sera plus intime ;)";
        server.handleClient(); 
        Serial.print("Position volet fermé ?  ");
        //volet fermé ?
        down();
        tempoMoteur = 0; //tempo moteur le temps que le moteur se lance
        while(valueCurrentMotorStandby < valueCurrentMotor){
          valueCurrentMotor = sensorStateCurrent();
        }
        measureCourse.pourcent = 50;
        measureCourse.info = "Volet fermé !  héhé :) <br /> J'ai peur dans le noir :(";
        server.handleClient(); 
        //volet fermé OK
        halt();
        Serial.println("OK");
        lightCurrentValue = valueCurrentMotorStandby + 500;
        valueCurrentMotor = lightCurrentValue;
        measureCourse.pourcent = 60;
        measureCourse.info = "J'ouvre le volet !<br />Mesure du temps de monté...du volet!";
        server.handleClient(); 
        Serial.println("Mesure temps de monté volet : Monte");
        up();
        tempoMoteur = 0; //tempo moteur le temps que le moteur se lance
        temporaryTime = millis();
        while(valueCurrentMotorStandby < valueCurrentMotor){
          valueCurrentMotor = sensorStateCurrent();
        }
        upCourseTime = millis()-temporaryTime;
        measureCourse.pourcent = 70;
        measureCourse.info = "Mesure du temps de monté terminé."; 
        halt();
        server.handleClient();
        Serial.print("Durée montée :");
        Serial.println(upCourseTime);        
        lightCurrentValue = valueCurrentMotorStandby + 500;
        valueCurrentMotor = lightCurrentValue;
        measureCourse.pourcent = 80;
        measureCourse.info = "Allez, on le referme ;)<br />Mesure du temps de descente...";
        server.handleClient();
        Serial.println("Mesure temps de descente volet : Descend");
        down();
        tempoMoteur = 0; //tempo moteur le temps que le moteur se lance
        temporaryTime = millis();
        while(valueCurrentMotorStandby < valueCurrentMotor){
          valueCurrentMotor = sensorStateCurrent();
        }
        downCourseTime = millis()-temporaryTime;
        measureCourse.pourcent = 90;
        measureCourse.info = "Mesure du temps de descente terminé.";
        halt();
        server.handleClient();
        Serial.print("Durée descente :");
        Serial.println(downCourseTime);
        Serial.println("Mesure terminée !");
        shutters.setCourseTime(upCourseTime, downCourseTime); 
        writeConfigFile(); 
        measureCourse.pourcent = 100;
        measureCourse.info = "Mesure terminée ! <br />Temps de monté : " + String(upCourseTime) + " ms / Temps de descente : " + String(downCourseTime) + " ms";     

      } else {
        sendHeaderAccess();
        server.send(404, "text/json", "{\"error\":\"Command error\"}");
      }
    }
  } else {
    sendHeaderAccess();
    server.send(404, "text/json", "{\"error\":\"Access denied\"}"); 
  }  
}

void stateAutoCourse(){
        sendHeaderAccess();
        String measureEnd = "false";
        String pourcent = String(measureCourse.pourcent) + "%";
        if(measureCourse.pourcent == 100){
          measureEnd = "true";
        }
        server.send(200, "text/json", "{\"pourcent\":\""+pourcent+"\",\"state\":\""+measureCourse.info+"\",\"end\":\""+measureEnd+"\"}" );
}

void sendState() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["motor"] = motorStatus;
  json["position"] = String(shutters.getCurrentLevel());
  String config_json;
  json.prettyPrintTo(config_json);  
  sendHeaderAccess();
  server.send(200, "text/json", config_json);
}

void sendStateToGladys (bool realState){
  HTTPClient http;
  String getData, link, payload;
  
  if(sensor && !debugMode){
    blinkLED(1, 1, 1, 0.2);
  } else {
    black();
  }
  
  getData = "?token=" + String(gladys_token) + "&devicetype=" + String(volet.id) + "&value=" + String(realState);
  link = "http://" + String(gladys_server) + ":" + String(gladys_port) + "/devicestate/create" + getData;
  http.begin(link);
  int httpCode = http.GET();
  if(httpCode >0) {
    payload = http.getString();
  }
  if(debugMode){
    Serial.println("Link: " + link);
    Serial.println("http code: " + httpCode);
    Serial.println("response: " + payload + "\n");
  }

  http.end();
  
  if(!debugMode){
    blinkLED(0,0,0,0.5);
  } else {
    blinkLED(0,1,0,0.1);
  }
}

void setup() {
  pinMode(BLUE_PIN_LED, OUTPUT);
  pinMode(RED_PIN_LED, OUTPUT);
  pinMode(GREEN_PIN_LED, OUTPUT);
  pinMode(REL_UP_PIN, OUTPUT);
  pinMode(REL_DOWN_PIN, OUTPUT);
  //init debug button
  pinMode(BUTTON_DEBUG_PIN, INPUT);
  
  black();
  halt();
  
  blinkLED(0, 1, 0, 0.5);

  button.setDebounceTicks(50);
  button.setClickTicks(500);
  button.setPressTicks(longPressDelay);
  button.attachDoubleClick(displayData);            // doubleclick display parameters on serial port.
  button.attachClick(debugState);                  // singleclick active debug mode.
  button.attachLongPressStart(resetConfig);            // reset wifi parameters
  
  Serial.begin(115200);
  delay(100);
  #ifdef ESP8266
  EEPROM.begin(512);
  #endif
  Serial.println();
  Serial.println("*** Starting ***");
  Serial.println("taille array :" + String(nbrGroup));
  // Mount the filesystem
  bool result = SPIFFS.begin();
  Serial.println("SPIFFS opened: " + result);
  initialNetworkParam = readConfigNetworkFile();
  if (!initialNetworkParam) {
    Serial.println("Failed to read network configuration file !!!");
    initialConfig = true;
  }
  
  WiFi.printDiag(Serial); //Remove this line if you do not want to see WiFi password printed
  if (WiFi.SSID() == "") {
    Serial.println("We haven't got any access point credentials, so get them now");
    initialConfig = true;
  } else {
    if (!initialConfig) {
      Serial.println("Init static IP :" + String(static_ip));
      if(static_ip != "" && strcmp(static_ip,"0.0.0.0") != 0) {
        //set static ip
        _ip.fromString(static_ip);
        _gw.fromString(static_gw);
        _sn.fromString(static_sn);
        _dns.fromString(static_dns);

        WiFi.config(_ip, _dns, _gw, _sn);
        Serial.println("Init static IP");
      }
      
      blinkLED(0, 1, 0, 0.5); // Turn LED off as we are not in configuration mode.
      WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.
      //WiFi.begin(SSID, password);
      unsigned long startedAt = millis();
      Serial.print("After waiting ");
      int connRes = WiFi.waitForConnectResult();
      float waited = (millis() - startedAt);
      Serial.print(waited / 1000);
      Serial.print(" secs in setup() connection result is ");
      Serial.println(connRes);
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    blinkLED(1, 0, 0, 0.5);
    Serial.println("Failed to connect, finishing setup anyway");
  } else {
    initialParam = readConfigFile();
    readLastPositionLevel();
    if(!initialParam) {
      blinkLED(0,0,1,0.1);
    } else {
      blinkLED(0,0,0,0.1);
      white();
    }
    //init INTERS
    pinMode(INTER_DOWN_PIN, INPUT);
    digitalWrite(INTER_DOWN_PIN, HIGH);
    pinMode(INTER_UP_PIN, INPUT);
    digitalWrite(INTER_UP_PIN, HIGH);
    interDown.attach(INTER_DOWN_PIN);
    interDown.interval(25);
    interUp.attach(INTER_UP_PIN);
    interUp.interval(25);
    
    Serial.print("Local ip: ");
    Serial.println(WiFi.localIP());
    //all the sites which will be available
    server.on( "/", getHeatindex );
    server.on( "/sendconfig", sendConfig );
    server.on( "/saveconfig", saveConfig );
    server.on( "/settingcourse", settingCourseTime );
    server.on( "/settingcourseauto", settingAutoCourseTime );
    server.on( "/readstatecourseauto", stateAutoCourse );
    server.on( "/sendstate", sendState );

    server.serveStatic("/js", SPIFFS, "/js");
    server.serveStatic("/css", SPIFFS, "/css");
    server.serveStatic("/fonts", SPIFFS, "/fonts");
    server.serveStatic("/sys", SPIFFS, "/index.html");
    server.serveStatic("/config", SPIFFS, "/config.html");
    server.serveStatic("/setupCourseTime", SPIFFS, "/setupCourseTime.html");
    server.serveStatic("/setupCourseTimeAuto", SPIFFS, "/setupCourseTimeAuto.html");
    server.serveStatic("/command", SPIFFS, "/gear.html");
    
    server.begin();
    espStart = 1;
    interUp.update();
    interDown.update();
    lastInterStatus = !(interUp.read())*10 + !(interDown.read());

    char storedShuttersState[shutters.getStateLength()];
    readInEeprom(storedShuttersState, shutters.getStateLength());
    shutters
      .setOperationHandler(shuttersOperationHandler)
      .setWriteStateHandler(shuttersWriteStateHandler)
      .restoreState(storedShuttersState)
      .setCourseTime(upCourseTime, downCourseTime)
      .onLevelReached(onShuttersLevelReached)
      .begin();

    if(onRestartGoToPosition == 0){
      shutters.setLevel(0); // Go to 0%
    }
    if(onRestartGoToPosition == 1){
      shutters.setLevel(100); // Go to 100%
    }
    if(onRestartGoToPosition == 2){
      if(lastPositionLevel > 100) {
        lastPositionLevel = 100;
      }
      shutters.setLevel(lastPositionLevel); // Go to ?%
    }
  }
}

void loop() {
  // is configuration portal requested?
  if (initialConfig) {
    clearConfig();
  }
  
  //process all the requests for the Webserver
  server.handleClient();  
  shutters.loop();
  interUD();
  button.tick();

  sensor = sensorStateLCR();
  if(!debugMode){
    if(initialParam){
      if(sensor){
        if(motorStatus == 1){
          if(ledStatus == false){ 
            blinkLED(1,0,1,0.5);
            ledStatus = true;
          }
        }
        if(motorStatus == 2){
          if(ledStatus == false){ 
            blinkLED(1,1,0,0.5);
            ledStatus = true;
          }
        }
        if(motorStatus == 0){
          ledStatus = false;
          blinkLED(0,0,0,0.5);
          white();
        }
      } else {
        ledStatus = false;
        blinkLED(0,0,0,0.5);
        black();
      }
    }
  }
}
