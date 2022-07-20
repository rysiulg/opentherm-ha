
//     https://maximeborges.github.io/esp-stacktrace-decoder/
//Nice Info about ESP: https://tttapa.github.io/ESP8266/Chap01%20-%20ESP8266.html


#define ATOMIC_FS_UPDATE   //For uploading compressed filesystems, the application must be built with ATOMIC_FS_UPDATE defined because, otherwise, eboot will not be involved in writing the filesystem.
//#define enableWebSerial     //not fully implemented
#define enableArduinoOTA
//#define enableWebUpdate
#define enableWebSocketlog  //send log to websocket
#define enableWebSocket
#define doubleResDet
#define wwwport 80


#include <ESPAsyncWebServer.h>
#ifdef ESP32
//#include <WiFi.h>
#ifdef enableWebUpdate
#include <Update.h>
#endif
#include <AsyncTCP.h>
#include <AsyncUDP.h>
#include <ESPmDNS.h>
#include <AsyncDNSServer.h>
#include "esp_task_wdt.h"
#else
#ifdef enableWebUpdate
#include <Updater.h>
#endif
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESP8266mDNS.h>
//#include <ESPAsyncUDP.h>
//#include <ESPAsyncDNSServer.h>
#endif


#ifdef enableMQTT
#include <PubSubClient.h>
#endif

#ifdef enableArduinoOTA
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#endif
#ifdef enableWebSerial
#include <WebSerial.h>
#endif
#ifdef ENABLE_INFLUX
  #ifdef ESP32
  #include <HTTPClient.h>
  #include <InfluxDbClient.h>
  #else
  #include <ESP8266HTTPClient.h>
  #include <InfluxDbClient.h>
  #endif
#endif
#ifdef doubleResDet
#include <ESP_DoubleResetDetector.h>
#endif
#include <DNSServer.h>
AsyncWebServer webserver(wwwport);
#include <ESPAsyncWebServer.h>  //also must be <ESPAsyncTCP.h> <ESP8266WiFi.h>
//#include <FS.h>                 //for spiffs files
#include "LittleFS.h" // LittleFS is declared
#define SPIFFS LittleFS       //4kB less after conversion but filesystem is higher allocation min 4kB fo LF and 256B for SPIFFS
#ifdef enableWebSocket
AsyncWebSocket WebSocket("/ws");
#endif
#ifdef ENABLE_INFLUX
InfluxDBClient InfluxClient(INFLUXDB_URL, INFLUXDB_DB_NAME);
Point InfluxSensor(InfluxMeasurments);
#endif
#ifdef doubleResDet
  #define DRD_TIMEOUT 0.1

  // address to the block in the RTC user memory
  // change it if it collides with another usageb
  // of the address block
  #define DRD_ADDRESS 0x00
  DoubleResetDetector* drd;
#endif
#ifdef enableMQTT
WiFiClient espClient;
PubSubClient mqttclient(espClient);
#endif
DNSServer dnsServer;

#ifndef decimalPlaces
#define decimalPlaces 1   //how much decimal places to show on www
#endif
//*********************************************************************************************************************************************
//         SPECIFIC functions outside from this file to get specific values
//*********************************************************************************************************************************************
#define ValuesToWSWPinJSON 1
#define ValuesToWSWPforWebProcessor 2
#ifndef ASS_Num
#define ASS_Num 21
#endif
typedef struct
{
  String Value;
} all_sensors_struct;
all_sensors_struct ASS[ASS_Num];
String get_PlaceholderName(u_int i);
void updateDatatoWWW_received(u_int i);
void updateDatatoWWW();
void mqttReconnect_subscribe_list();

//*********************************************************************************************************************************************

const char version[10+1] =
{
   // YY year
   //__DATE__[7], __DATE__[8],
   __DATE__[9], __DATE__[10],

   // First month letter, Oct Nov Dec = '1' otherwise '0'
   (__DATE__[0] == 'O' || __DATE__[0] == 'N' || __DATE__[0] == 'D') ? '1' : '0',

   // Second month letter
   (__DATE__[0] == 'J') ? ( (__DATE__[1] == 'a') ? '1' :       // Jan, Jun or Jul
                            ((__DATE__[2] == 'n') ? '6' : '7') ) :
   (__DATE__[0] == 'F') ? '2' :                                // Feb
   (__DATE__[0] == 'M') ? (__DATE__[2] == 'r') ? '3' : '5' :   // Mar or May
   (__DATE__[0] == 'A') ? (__DATE__[1] == 'p') ? '4' : '8' :   // Apr or Aug
   (__DATE__[0] == 'S') ? '9' :                                // Sep
   (__DATE__[0] == 'O') ? '0' :                                // Oct
   (__DATE__[0] == 'N') ? '1' :                                // Nov
   (__DATE__[0] == 'D') ? '2' :                                // Dec
   0,

   // First day letter, replace space with digit
   __DATE__[4]==' ' ? '0' : __DATE__[4],

   // Second day letter
   __DATE__[5],
   __TIME__[0],__TIME__[1],
   __TIME__[3],__TIME__[4],
  '\0'
};


uint8_t mac[6] = {(uint8_t)strtol(WiFi.macAddress().substring(0,2).c_str(),0,16), (uint8_t)strtol(WiFi.macAddress().substring(3,5).c_str(),0,16),(uint8_t)strtol(WiFi.macAddress().substring(6,8).c_str(),0,16),(uint8_t)strtol(WiFi.macAddress().substring(9,11).c_str(),0,16),(uint8_t)strtol(WiFi.macAddress().substring(12,14).c_str(),0,16),(uint8_t)strtol(WiFi.macAddress().substring(15,17).c_str(),0,16)};

const unsigned long mqttUpdateInterval_ms = 1 * 60 * 1000,      //send data to mqtt and influxdb
                    LOOP_WAITTIME = (2.1*1000),    //for loop
                    WIFIRETRYTIMER = 25 * 1000;



unsigned long uptime = 0,
              lastloopRunTime = 0,        //for loop
              lastUpdatemqtt = 0,
              lastWifiRetryTimer = 0;

bool starting = true,
     sendlogtomqtt = false,       //Send or not Logging to MQTT Topic
     receivedmqttdata = false,
     receivedwebsocketdata = false,
     firstConnectSinceBoot = false;

#ifndef ecoModeMaxTemp
#define ecoModeMaxTemp 39
#endif
#ifndef ecoModeDisabledMaxTemp
#define ecoModeDisabledMaxTemp 60
#endif

float opcohi = ecoModeDisabledMaxTemp,             // upper max heat boiler to CO
      ecohi = ecoModeMaxTemp;

const float ophi = 65,               // upper max heat water
            opcohistatic = opcohi,
            oplo = 29,               // lower min heat water
            opcolo = oplo,           // lower min heat boiler to CO
            cutoffhi = 20,           // upper max cut-off temp above is heating CO disabled -range +-20
            cutofflo = -cutoffhi,    // lower min cut-off temp above is heating CO disabled
            roomtemphi = 29,         // upper max to set of room temperature

            roomtemplo = 15;         // lower min to set of room temperature

char log_chars[256];      //for logging buffer to log_message function
int mqttReconnects = -1,
    temp_NEWS_count = 0,
    mqtt_offline_retrycount = 0,
    mqtt_offline_retries = 2; // retries to mqttconnect before timewait
size_t content_len;

//other_Modules

char ssid[sensitive_size] = SSID_Name;
char pass[sensitive_size] = SSID_PAssword;

// Your MQTT broker address and credentials
char mqtt_server[sensitive_size * 2] = MQTT_servername;
char mqtt_user[sensitive_size] = MQTT_username;
char mqtt_password[sensitive_size] = MQTT_Password_data;
int mqtt_port = MQTT_port_No;
const int mqtt_Retain = 1;

//common_functions.h
bool check_isValidTemp(float temptmp);
void log_message(char* string, u_int specialforce);
String uptimedana(unsigned long started_local, bool startFromZero);
String getJsonVal(String json, String tofind);
bool isValidNumber(String str);
String convertPayloadToStr(byte *payload, unsigned int length);
int dBmToQuality(int dBm);
int getWifiQuality();
int getFreeMemory();
bool PayloadStatus(String payloadStr, bool state);
bool PayloadtoValidFloatCheck(String payloadStr);
float PayloadtoValidFloat(String payloadStr,bool withtemps_minmax=false, float mintemp=-InitTemp, float maxtemp=InitTemp);
void restart();
String getIdentyfikator(int x);
#ifdef enableArduinoOTA
void Setup_OTA();
#endif
char* dtoa(double dN, char *cMJA, int iP);
#ifdef enableMESHNETWORK
void MeshWiFi_sendMessage();
void Setup_MeshWiFi();
#endif
void Setup_WiFi();
void Setup_Influx();
void Setup_DNS();
void Setup_FileSystem();
#ifdef enableWebSocket
void notifyClients(String Message);
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void Event_WebSocket(AsyncWebSocket *webserver, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void Setup_WebSocket();
#endif
String web_processor(const String var);
bool webhandleFileRead(AsyncWebServerRequest *request, String path);
String getValuesToWebSocket_andWebProcessor(u_int function, String processorAsk = "\0");
void handleWebSocketMessage_sensors(String message);
void Setup_WebServer();
void webhandleNotFound(AsyncWebServerRequest *request);
void webhandleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void MainCommonSetup();
void MainCommonLoop();
void Setup_Mqtt();
void mqttReconnect();
String formatBytes(size_t bytes);
String webgetContentType(String filename);
void check_wifi();
#ifdef enableWebUpdate
void SetupWebUpdate();
void handleUpdate(AsyncWebServerRequest *request);
void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) ;
void printProgress(size_t prg, size_t sz);
#endif
String PrintHex8(const uint8_t *data, char separator, uint8_t length);

//***********************************************************************************************************************************************************************************************
bool check_isValidTemp(float temptmp)
{

  #define DS18B20nodata 85
  #define DS18B20nodata1 127
  if (temptmp!=InitTemp && temptmp!=-InitTemp && temptmp!=DS18B20nodata && temptmp!=-DS18B20nodata && temptmp!=DS18B20nodata1 && temptmp!=-DS18B20nodata1) return true; else return false;
  #undef DS18B20nodata
  #undef DS18B20nodata1

}
//***********************************************************************************************************************************************************************************************
void log_message(char* string, u_int specialforce = 0)  //         log_message((char *)"WiFi lost, but softAP station connecting, so stop trying to connect to configured ssid...");
{
  #include "configmqtttopics.h"
  String send_string = String(millis()) + F(": ") + String(string);
  //free(string);
  #ifdef debugSerial
    Serial.println(send_string);
  #endif
  #ifdef enableWebSerial
    if (send_string.length() > 255) send_string[255] = '\0';
    if (starting == false) {WebSerial.println(send_string);}
  #endif
//   if (webSocket.connectedClients() > 0) {
//     webSocket.broadcastTXT(string, strlen(string));
//   }
  #ifdef enableMQTT
  if (sendlogtomqtt || specialforce == 2) {
    if (mqttclient.connected() && starting == false)
    {
      if (send_string.length() > 255) send_string[255] = '\0';
      if (!mqttclient.publish(String(LOG_TOPIC).c_str(), send_string.c_str())) {
        Serial.print(millis());
        Serial.print(F(": "));
        Serial.println(F("MQTT publish log message failed!"));
        mqttclient.disconnect();
      }
    }
  }
  #endif
  #ifdef enableWebSocketlog
  send_string.replace("\"","\\\"");
  send_string.replace("\\\\","\\");
    notifyClients(String("{\"log\":\""+String(send_string)+"\"}").c_str());
  #endif

}

//***********************************************************************************************************************************************************************************************
String uptimedana(unsigned long started_local = 0, bool startFromZero = false) {
  String wynik = "\0";
  unsigned long  partia;
  if (startFromZero) partia = started_local; else partia = millis() - started_local;
  if (partia<1000) return "< 1 "+String(t_sek)+" ";
  // #ifdef debug2
  //   Serial.print(F("Uptimedana: "));
  // #endif
  if (partia >= 24 * 60 * 60 * 1000 ) {
    unsigned long  podsuma = partia / (24 * 60 * 60 * 1000);
    partia -= podsuma * 24 * 60 * 60 * 1000;
    wynik += (String)podsuma + ""+String(t_day)+" ";
  }
  if (partia >= 60 * 60 * 1000 ) {
    unsigned long  podsuma = partia / (60 * 60 * 1000);
    partia -= podsuma * 60 * 60 * 1000;
    wynik += (String)podsuma + ""+String(t_hour)+" ";
  }
  if (partia >= 60 * 1000 ) {
    unsigned long  podsuma = partia / (60 * 1000);
    partia -= podsuma * 60 * 1000;
    wynik += (String)podsuma + ""+String(t_min)+" ";
    //Serial.println(podsuma);
  }
  if (partia >= 1 * 1000 ) {
    unsigned long  podsuma = partia / 1000;
    partia -= podsuma * 1000;
    wynik += (String)podsuma + ""+String(t_sek)+" ";
    //Serial.println(podsuma);
  }
  //wynik += (String)partia + "/1000";  //pomijam to wartosci <1sek
  return wynik;
}

//***********************************************************************************************************************************************************************************************
String getJsonVal(String json, String tofind)
{ //function to get value from json payload
  json.trim();
  tofind.trim();
  #ifdef debugweb
  sprintf(log_chars,"json0: %s",json.c_str());
  log_message(log_chars);
  #endif
  if (!json.isEmpty() and !tofind.isEmpty() and json.startsWith("{") and json.endsWith("}"))  //check is starts and ends as json data and nmqttident null
  {
    if (json.indexOf(tofind)>=0 )
    {
      //Found string and get value
      u_int valu_start_position = json.indexOf(":", json.indexOf(tofind)) + 1;
      u_int valu_end_position = json.indexOf(",", valu_start_position);

      String nvalue=json.substring(valu_start_position, valu_end_position); //get node value
      nvalue.trim();
      #ifdef debugweb
      sprintf(log_chars,"Found node: %s, return val: %s", String(tofind).c_str(), String(nvalue).c_str());
      log_message(log_chars);
      #endif
      return nvalue;
    }
    sprintf(log_chars, "Json %s, No mqttident contain searched value of %s", json.c_str(), tofind.c_str());
    log_message(log_chars);  //3150232: Json "mqtt reconnects":53, No mqttident contain searched value of room

  } else
  {
    sprintf(log_chars, "Inproper Json format or null: %s, to find: %s", json.c_str(), tofind.c_str());
    log_message(log_chars);
  }
  return "\0";
}

//***********************************************************************************************************************************************************************************************
String getJsonVal_old(String json, String tofind)
{ //function to get value from json payload
  json.trim();
  tofind.trim();
  #ifdef debugweb
  sprintf(log_chars,"json0: %s",json.c_str());
  log_message(log_chars);
  #endif
  if (!json.isEmpty() and !tofind.isEmpty() and json.startsWith("{") and json.endsWith("}"))  //check is starts and ends as json data and nmqttident null
  {
    json=json.substring(1,json.length()-1);                             //cut start and end brackets json
    if (json.indexOf("{",1) !=-1)
    {
      json.replace("{","\"jsonskip\":\"0\",");      //was "{","\"jsonskip\",");
      json.replace("}","");
    }
    #ifdef debugweb
    sprintf(log_chars,"json1: %s",json.c_str());
    log_message(log_chars);
    #endif
    int tee=0; //for safety ;)
    #define maxtee 500
    while (tee!=maxtee)
    {         //parse all nodes
      int pos = json.indexOf(",",1);                //position to end of node:value
      if (pos==-1) {tee=maxtee;}
      String part;
      if (pos>-1) {part = json.substring(0,pos);} else {part=json; }       //extract parameter node:value
      part.replace("\"","");                      //clean from text indent
      part.replace("'","");
      json=json.substring(pos+1);                      //cut input for extracted node:value
      if (part.indexOf(":",1)==-1) {
        #ifdef debugweb
          sprintf(log_chars,"Return no data");
          log_message(log_chars);
        #endif
        break;
      }
      String node=part.substring(0,part.indexOf(":",1));    //get node name
      node.trim();
      String nvalue=part.substring(part.indexOf(":",1)+1); //get node value
      nvalue.trim();
      #ifdef debugweb
      sprintf(log_chars,"jsonx: %s, tee: %s, tofind: %s, part: %s, node: %s, nvalue: %s, indexof ,: %s",String(json).c_str(), String(tee).c_str(), String(tofind).c_str(), String(part).c_str(), String(node).c_str(), String(nvalue).c_str(), String(json.indexOf(",",1)).c_str());
      log_message(log_chars);
      #endif
      if (tofind.indexOf(node) >=0 )
      {
         nvalue.replace("\"","");
         nvalue.replace(",",".");   //to get valid decimal number
         #ifdef debugweb
         sprintf(log_chars,"Found node: %s, return val: %s", String(tofind).c_str(), String(nvalue).c_str());
         log_message(log_chars);
         #endif
        return nvalue;
        break;
      }
      tee++;
      if (tee>maxtee) {
        #ifdef debugweb
        sprintf(log_chars,"tee exit: %s", String(tee).c_str());
        log_message(log_chars);
        #endif
        break;  //safety bufor
      }
    }
    sprintf(log_chars, "Json %s, No mqttident contain searched value of %s", json.c_str(), tofind.c_str());
    log_message(log_chars);  //3150232: Json "mqtt reconnects":53, No mqttident contain searched value of room

  } else
  {
    sprintf(log_chars, "Inproper Json format or null: %s, to find: %s", json.c_str(), tofind.c_str());
    log_message(log_chars);
  }
  return "\0";
}

//***********************************************************************************************************************************************************************************************
bool isValidNumber(String str)
{
  bool valid = true;
  for (byte i = 0; i < str.length(); i++)
  {
    char ch = str.charAt(i);
    valid &= isDigit(ch) ||
             ch == '+' || ch == '-' || ch == ',' || ch == '.' ||
             ch == '\r' || ch == '\r';
  }
  return valid;
}

//***********************************************************************************************************************************************************************************************
String convertPayloadToStr(byte *payload, u_int length)
{
  char s[length + 1];
  s[length] = 0;
  for (u_int i = 0; i < length; ++i)
    s[i] = payload[i];
  String tempRequestStr(s);
  return tempRequestStr;
}

//***********************************************************************************************************************************************************************************************
int dBmToQuality(int dBm) {
  if (dBm <= -100)
    return 0;
  if (dBm >= -50)
    return 100;
  return 2 * (dBm + 100);
}

//***********************************************************************************************************************************************************************************************
int getWifiQuality() {
  if (WiFi.status() != WL_CONNECTED)
    return -1;
  return dBmToQuality(WiFi.RSSI());
}

//***********************************************************************************************************************************************************************************************
int getFreeMemory() {
  //store total memory at boot time
  static uint32_t total_memory = 0;
  if ( 0 == total_memory ) total_memory = ESP.getFreeHeap();

  uint32_t free_memory   = ESP.getFreeHeap();
  return (100 * free_memory / total_memory ) ; // as a %
}

//***********************************************************************************************************************************************************************************************
bool PayloadStatus(String payloadStr, bool state)
{
  payloadStr.toUpperCase();
  payloadStr.trim();
  if (state and (payloadStr == "ON" or payloadStr == "TRUE" or payloadStr == "START" or payloadStr == "1"  or payloadStr == "ENABLE" or payloadStr == "HEAT")) return true;
  else
  if (!state and (payloadStr == "OFF" or payloadStr == "FALSE" or payloadStr == "STOP" or payloadStr == "0" or payloadStr == "DISABLE")) return true;
  else return false;
}

//***********************************************************************************************************************************************************************************************
bool PayloadtoValidFloatCheck(String payloadStr)
{
  if (isValidNumber(payloadStr)) return true; else return false;
}

//***********************************************************************************************************************************************************************************************
float PayloadtoValidFloat(String payloadStr, bool withtemps_minmax, float mintemp, float maxtemp)  //bool withtemps_minmax=false, float mintemp=InitTemp,float
{
  payloadStr.trim();
  if (isValidNumber(payloadStr))  payloadStr.replace(",", ".");
  float valuefromStr = payloadStr.toFloat();
  if (isnan(valuefromStr) || !isValidNumber(payloadStr))
  {
    #ifdef debug
    sprintf(log_chars, "Value is not a valid number, ignoring... payload: %s", payloadStr.c_str());
    log_message(log_chars);
    #endif
    return InitTemp;
  } else
  {
    if (!withtemps_minmax)
    {
      // sprintf(log_chars, "Value is valid number without minmax: %s", String(valuefromStr,2).c_str());
      // log_message(log_chars);
      // return valuefromStr;
    } else {
      if (valuefromStr>maxtemp and !check_isValidTemp(maxtemp)) valuefromStr = maxtemp;
      if (valuefromStr<mintemp and !check_isValidTemp(mintemp)) valuefromStr = mintemp;
      #ifdef debug
      sprintf(log_chars, "Value is valid number: %s", String(valuefromStr,2).c_str());
      log_message(log_chars);
      #endif
    }
    if (!check_isValidTemp(valuefromStr)) valuefromStr = InitTemp;
    return valuefromStr;
    //if here is sprintf and return -i have dpouble reading because first i check is ok and second get value
  }
}

//***********************************************************************************************************************************************************************************************
void restart()
{
  delay(1500);
//  WiFi.forceSleepBegin();
  webserver.end();
  WiFi.disconnect();
//      wifi.disconnect();
  delay(5000);
//  WiFi.forceSleepBegin(); wdt_reset();
ESP.restart(); while (1)ESP.restart();
//wdt_reset();
ESP.restart();
}

//***********************************************************************************************************************************************************************************************
String getIdentyfikator(int x)
{
  return "_"+String(x+1);
}

//***********************************************************************************************************************************************************************************************
void doubleResetDetect() {
  #ifdef doubleResDet
  drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
  if (drd->detectDoubleReset())
  {
    Serial.println(F("DRD"));
    #ifdef enableWebSocket
    SPIFFS.begin();
    SPIFFS.format();
    #endif
    CONFIGURATION.version[0] = 'R';
    CONFIGURATION.version[1] = 'E';
    CONFIGURATION.version[2] = 'S';
    CONFIGURATION.version[3] = 'E';
    CONFIGURATION.version[4] = 'T';
    SaveConfig();
    WiFi.persistent(true);
    WiFi.disconnect();
    WiFi.persistent(false);
    #ifdef enableArduinoOTA
    Setup_WiFi();
    Setup_OTA();
    Setup_DNS();
    #ifdef enableWebUpdate
    SetupWebUpdate();
    Setup_WebServer();
    webserver.begin();
    #endif
    while (true) {
      ArduinoOTA.handle();
      //yield();
      check_wifi();
    }
    #endif
    delay(4000);
    restart();

  }
  #endif
}

//***********************************************************************************************************************************************************************************************
#ifdef enableArduinoOTA
void Setup_OTA() {
  log_message((char*)"Setup Arduino OTA...");
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(String(me_lokalizacja).c_str());

  // Set authentication
  //ArduinoOTA.setPassword("admin");

  ArduinoOTA.onStart([]() {
  #ifdef enableWebSocket
   // Disable client connections
   WebSocket.enable(false);
   // Advertise connected clients what's going on
   notifyClients(F("OTA Update Started"));
   // Close them
   WebSocket.closeAll();
   webserver.end();
  #endif
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }
    sprintf(log_chars,"Starting update by OTA type: %s", type.c_str());
    log_message(log_chars);
  });
  ArduinoOTA.onEnd([]() {
    log_message((char*)F("Finished update by OTA. Restart in 2sec..."),1);
    delay(2000);
    restart();

  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    if ((int)((progress/total)*100) % 5 == 0)
    {
      sprintf(log_chars,"Update in progress (Total %s/%s bytes).  %s%%",String(progress).c_str(), String(total).c_str(), String(float((float)progress/(float)total)*100,0).c_str());
      log_message(log_chars);
    }
  });
  ArduinoOTA.onError([](ota_error_t error) {
    String errorName = "\0";
    if (error == OTA_AUTH_ERROR) { errorName = F("Auth Failed") ;
    } else if (error == OTA_BEGIN_ERROR) { errorName = F("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) { errorName = F("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) { errorName = F("Receive Failed");
    } else if (error == OTA_END_ERROR) {errorName = F("End Failed");
    }
    webserver.begin();
    sprintf(log_chars,"There is error upgrading by OTA: %s (%s)",String(error).c_str(), String(errorName).c_str());
    log_message(log_chars);
  });
  ArduinoOTA.begin();
}
#endif
//***********************************************************************************************************************************************************************************************
//Ex.) char cVal[10];  float fVal=((22.0 /7.0)*256)-46.85;
// dtoa(fVal,cVal,4); Serial.println (String(cVal));

  //arguments...
  // float-double value, char array to fill, precision (4 is .xxxx)
  //and... it rounds last digit
char* dtoa(double dN, char *cMJA, int iP) {
  char *ret = cMJA; long lP=1; byte bW=iP;
  while (bW>0) { lP=lP*10;  bW--;  }
  long lL = long(dN); double dD=(dN-double(lL))* double(lP);
  if (dN>=0) { dD=(dD + 0.5);  } else { dD=(dD-0.5); }
  long lR=abs(long(dD));  lL=abs(lL);
  if (lR==lP) { lL=lL+1;  lR=0;  }
  if ((dN<0) & ((lR+lL)>0)) { *cMJA++ = '-';  }
  ltoa(lL, cMJA, 10);
  if (iP>0) { while (*cMJA != '\0') { cMJA++; } *cMJA++ = '.'; lP=10;
  while (iP>1) { if (lR< lP) { *cMJA='0'; cMJA++; } lP=lP*10;  iP--; }
  ltoa(lR, cMJA, 10); }  return ret; }

//***********************************************************************************************************************************************************************************************

#ifdef enableMESHNETWORK //painlessmesh  TaskScheduler  ESPAsyncTCP
#ifndef MESH_PREFIX
#define MESH_PREFIX     "Meshname"
#endif
#ifndef MESH_PASSWORD
#define MESH_PASSWORD   "meshpassword"
#endif
#ifndef MESH_PORT
#define MESH_PORT       5555 // default port
#endif

#include "painlessMesh.h"

void MeshWiFi_sendMessage();

Scheduler MeshWiFi_userScheduler;
painlessMesh  MeshWiFi;
Task MeshWiFi_taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &MeshWiFi_sendMessage );

void MeshWiFi_sendMessage()
{
  log_message((char*)F("Start Sending Mesh msg...."));
  // Serializing in JSON Format
  DynamicJsonDocument doc(1024);
//  float h = dht.readHumidity();
//  float t = dht.readTemperature();
//  doc["TEMP"] = t;
//  doc["HUM"] = h;
  doc["IP"] = WiFi.localIP();
  String msg ;
  serializeJson(doc, msg);

  MeshWiFi.sendBroadcast( msg );
  sprintf(log_chars,"Mesh Message Send: %s", msg.c_str());
  log_message(log_chars);
  MeshWiFi_taskSendMessage.setInterval((TASK_SECOND * 1, TASK_SECOND * 10));
}
// Needed for painless library
void MeshWiFi_receivedCallback( uint32_t from, String &msg ) {
  //Whenever there will be a message in the network, receivedCallback() comes in the action. The receivedCallback() function prints the message sender (from) and the content of the message using msg.c_str(). As data is serialized in sending function, it must be de-serialized while receiving.
  sprintf(log_chars,"Mesh Message Received: %s", msg.c_str());
  log_message(log_chars);
  String json;
  DynamicJsonDocument doc(1024);
  json = msg.c_str();
  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    sprintf(log_chars,"deserialize Json failed:: %s", error.c_str());
    log_message(log_chars);
  }
//  LState = doc["Button"];
//  digitalWrite(LED, LState);
}
void MeshWiFi_newConnectionCallback(uint32_t nodeId) {
  //the newConnectionCallback() runs whenever a new node joins the network. This function prints the chip ID of the new node.
  sprintf(log_chars,"Mesh --> startHere: New Connection, nodeId = %u\n", nodeId);
  log_message(log_chars);
}
void MeshWiFi_changedConnectionCallback() {
  //The changedConnectionCallback() runs whenever a connection changes on the network that is when a node joins or leaves the network.
  sprintf(log_chars,"Mesh Changed connections %s", "\0");
  log_message(log_chars);
}
void MeshWiFi_nodeTimeAdjustedCallback(int32_t offset) {
  //The nodeTimeAdjustedCallback() runs when the network adjusts the time, so that all nodes are synchronized.
  sprintf(log_chars,"Mesh Adjusted time: %u. Offset = %d\n", MeshWiFi.getNodeTime(), offset);
  log_message(log_chars);
}

//***********************************************************************************************************************************************************************************************
void Setup_MeshWiFi()
{
  MeshWiFi.setDebugMsgTypes( ERROR | STARTUP );
  //Here, debug function is used to get any error occurred while making a connection with the nodes.
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | MSG_TYPES | REMOTE ); // all types on except GENERAL
  MeshWiFi.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA);
  MeshWiFi.onReceive(&MeshWiFi_receivedCallback);
  //When a new node make connection this callback is called.
  MeshWiFi.onNewConnection(&MeshWiFi_newConnectionCallback);
  //When there is a change in mesh topology this callback is called.
  MeshWiFi.onChangedConnections(&MeshWiFi_changedConnectionCallback);
  //Enable the taskSendMessage to start sending the messages to the mesh.
  MeshWiFi.onNodeTimeAdjusted(&MeshWiFi_nodeTimeAdjustedCallback);
  //In this, scheduler executes the task and send the appropriate message to nodes.
  MeshWiFi.stationManual(SSID_Name, SSID_PAssword);
  MeshWiFi.setHostname(me_lokalizacja.c_str());
  MeshWiFi_userScheduler.addTask( MeshWiFi_taskSendMessage );
  MeshWiFi_taskSendMessage.enable();
}
#endif
void Setup_WiFi()
{
  sprintf(log_chars,"SetupWiFi...Connecting to: %s", String(ssid).c_str());
  log_message(log_chars);
  #ifndef ESP32
  WiFi.hostname(String(me_lokalizacja).c_str());      //works for esp8266
  #else
//  setCpuFrequencyMhz(80);   //STANDARD 240mHz
  WiFi.disconnect(true);
  WiFi.config(((u32_t)0x0UL),((u32_t)0x0UL),((u32_t)0x0UL));//IPADDR_NONE, INADDR_NONE, INADDR_NONE); //none gives 255.255.255.255 error in libraries
  WiFi.setHostname((me_lokalizacja).c_str());  //for esp32
  #endif
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(String(me_lokalizacja).c_str());

  WiFi.begin(ssid, pass);
  WiFi.enableSTA(true);
  WiFi.setAutoReconnect(true);
  WiFi.setAutoConnect(true);
  WiFi.persistent(true);
  sprintf(log_chars,"SSID: %s  PASS: %s", String(ssid).c_str(), String(pass).c_str());
  log_message(log_chars);

  int deadCounter = 20;
  while (WiFi.status() != WL_CONNECTED && deadCounter-- > 0)
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    sprintf(log_chars,"Failed to connect to : %s", String(ssid).c_str());
    log_message(log_chars);
    while (true); //this restart
  }
  else
  {
    sprintf(log_chars,"OK ");
    log_message(log_chars);
  }
  sprintf(log_chars,"Hostname: %s, LocalIP: %s", String(WiFi.getHostname()).c_str(), WiFi.localIP().toString().c_str());
  log_message(log_chars);
}
void Setup_Influx()
{
#ifdef ENABLE_INFLUX
  // InfluxDB
  InfluxClient.setConnectionParamsV1(INFLUXDB_URL, INFLUXDB_DB_NAME, INFLUXDB_USER, INFLUXDB_PASSWORD);
  // Alternatively, set insecure connection to skip server certificate validation
  InfluxClient.setInsecure();
  InfluxClient.setHTTPOptions(HTTPOptions().httpReadTimeout(3000));
  InfluxClient.setWriteOptions(WriteOptions().bufferSize(1024));
  InfluxClient.setWriteOptions(WriteOptions().retryInterval(5));
  InfluxClient.setWriteOptions(WriteOptions().maxRetryInterval(300));
  InfluxClient.setWriteOptions(WriteOptions().maxRetryAttempts(3));

  // Add tags
  InfluxSensor.addTag("device", me_lokalizacja);
  // Check server connection
  if (InfluxClient.validateConnection())
  {
    sprintf(log_chars, "Connected to InfluxDB: %s", String(InfluxClient.getServerUrl()).c_str());
    log_message(log_chars,0);
  }
  else
  {
    sprintf(log_chars, "InfluxDB connection failed: %s", String(InfluxClient.getLastErrorMessage()).c_str());
    log_message(log_chars,0);
  }
#endif
}
//***********************************************************************************************************************************************************************************************
void Setup_DNS()
{
  log_message((char*)F("Setup DNS..."));
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.localIP());
  MDNS.begin(String(me_lokalizacja).c_str());     // start the multicast domain name server
  MDNS.addService("http", "tcp", 80);
}
//***********************************************************************************************************************************************************************************************
void Setup_FileSystem()
{// Start the FSand list all contents
  SPIFFS.begin();                             // Start the SPI Flash File System (SPIFFS)
  log_message((char*)F("FS started. Contents:"));
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {                      // List the file system contents
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      sprintf(log_chars, "\tFS File: %s, size: %s", fileName.c_str(), formatBytes((size_t)fileSize).c_str());
      log_message(log_chars);
    }
    //
  }
}
//***********************************************************************************************************************************************************************************************
#ifdef enableWebSocket
void notifyClients(String Message)
{
  if (WebSocket.availableForWriteAll() && WebSocket.enabled()) WebSocket.textAll(Message);
}
//***********************************************************************************************************************************************************************************************
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
      data[len] = 0;
      String message = (char*)data;
      message.trim();
      if (strcmp((char*)data, "toggle") == 0) {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        notifyClients(digitalRead(LED_BUILTIN) ? "on" : "off");       //notify
      } else
      {
        handleWebSocketMessage_sensors(message);
      }
    }
}
//***********************************************************************************************************************************************************************************************
void Event_WebSocket(AsyncWebSocket       *webserver,  //
             AsyncWebSocketClient *client,  //
             AwsEventType          type,    // the signature of this function is defined
             void                 *arg,     // by the `AwsEventHandler` interface
             uint8_t              *data,    //
             size_t                len) {   // uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) { // When a WebSocket message is received
  //WS_EVT_CONNECT when a client has logged in,
  // WS_EVT_DISCONNECT when a client has logged out,
  // WS_EVT_DATA when a data packet is received from the client.
  // WS_EVT_PONG in response to a ping request,
  // WS_EVT_ERROR when an error is received from the client,


  #ifdef enableWebSocket
  switch (type) {
    case WS_EVT_DISCONNECT:             // if the websocket is disconnected
      sprintf(log_chars, "[%s] Disconnected!\n", String(client->id()).c_str());
      log_message(log_chars);
      break;
    case WS_EVT_CONNECT: {              // if a new websocket connection is established
        IPAddress ip = client->remoteIP();
        sprintf(log_chars,"[%u] Connected from %d.%d.%d.%d", (u_int)client->id(), ip[0], ip[1], ip[2], ip[3]);
        log_message(log_chars);
        // rainbow = false;                  // Turn rainbow off when a new connection is established
      }
      break;
    case WS_EVT_DATA:                     // if new text data is received
      sprintf(log_chars, "[%s] get Text WS_EVT_DATA.", String(client->id()).c_str()); //String(*data).c_str());
      log_message(log_chars);
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
      log_message((char*)F("WS_EVT_PONG received"));
      break;
    case WS_EVT_ERROR:
      log_message((char*)F("WS_EVT_ERROR received"));
      break;
  }
  #endif
}
//***********************************************************************************************************************************************************************************************
void Setup_WebSocket()
{ // Start a WebSocket server
  WebSocket.onEvent(Event_WebSocket);          // if there's an incomming websocket message, go to function 'webSocketEvent'
  webserver.addHandler(&WebSocket);             // start the websocket server
  log_message((char*)F("WebSocket server started."));
}
//***********************************************************************************************************************************************************************************************
#endif
String web_processor(const String var) {
  // sprintf(log_chars,"WebProcessor variable: %s",var.c_str());
  // log_message(log_chars);
  if (var == "ME_TITLE") {return String(me_lokalizacja) + " v." + String(version);
  } else
  if (var == "DIR_LIST") {
    String retval = "FS started. Contents:";
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {                      // List the file system contents
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      retval += F("\n<br>\tFS File: ");
      retval += String(fileName);
      retval += F(", size: ");
      retval += formatBytes((size_t)fileSize);
    }
    return retval;
  } else
  if (var == "stopkawebsite") {
    String ptr="\0";
    const String  stopka = String(MFG)+" "+version[4]+version[5]+"-"+version[2]+version[3]+"-20"+version[0]+version[1]+" "+version[6]+version[7]+":"+version[8]+version[9];
      #ifdef enableWebUpdate
      ptr += F("<p><br><span class='units'><a href='/update' target=\"_blank\">")+String(Update_web_link)+F("</a>");
      #endif
      #ifdef enableWebSerial
      ptr += F("&nbsp; &nbsp;&nbsp; <a href='/webserial' target=\"_blank\">")+String(Web_Serial)+F("</a>");
      #endif
      ptr += F("&nbsp;&nbsp;&nbsp;<a href='/edit'>")+String("Edit FileSystem")+F("</a>&nbsp;");
      ptr += F("</p><br/>");
      ptr += F("MAC: <B>");
      ptr += PrintHex8(mac, ':', sizeof(mac) / sizeof(mac[0]));

      ptr += F("</B>, CRT: <B>");
      ptr += String(runNumber);
      ptr += F("</B><br>");
      ptr += F("<p style=\"color:darkblue;font-size:1.1rem;\">&copy; ");
      ptr += stopka;
      ptr += F("</p>");
    return String(ptr);
  } else
  if (var == "opcolo") { return String(opcolo);
  } else
  if (var == "opcohi") { return String(opcohi);
  } else
  if (var == "oplo") { return String(oplo);
  } else
  if (var == "ophi") { return String(ophi);
  } else
  if (var == "cutofflo") { return String(cutofflo);
  } else
  if (var == "cutoffhi") { return String(cutoffhi);
  } else
  if (var == "roomtemplo") { return String(roomtemplo);
  } else
  if (var == "roomtemphi") { return String(roomtemphi);
  } else
  if (var == "STATE") { return (digitalRead(LED_BUILTIN) ? "on" : "off");
  } else
  {
    return getValuesToWebSocket_andWebProcessor(ValuesToWSWPforWebProcessor, var);          //get specific data of sensors
  }

}




//***********************************************************************************************************************************************************************************************
bool webhandleFileRead(AsyncWebServerRequest *request, String path){
  sprintf(log_chars,"handleFileRead: %s", path.c_str());
  log_message(log_chars);

  // if (!request->authenticate(www_username, www_password)) {
  //     Serial.print(F("NOT AUTHENTICATE!"));
  //     request->requestAuthentication();
  // }

  if(path.endsWith("/") and path.length()<2) path += F("index.html");           // If a folder is requested, send the index file
  String contentType = webgetContentType(path);                // Get the MIME type
  String pathWithGz = path + F(".gz");
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){     // If the file exists, either as a compressed archive, or normal
    bool gzipped = false;

    if(SPIFFS.exists(pathWithGz)) {                         // If there's a compressed version available
      path += F(".gz");                                     // Use the compressed version
      gzipped = true;
    }
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, path, contentType);
    if (gzipped){
        response->addHeader("Content-Encoding", "gzip");
    }
    sprintf(log_chars,"Real file path: %s",String(path).c_str());
    log_message(log_chars);

    request->send(response);

    return true;
  }
  sprintf(log_chars,"File Not Found: %s",String(path).c_str());
  log_message(log_chars);
  return false;                                             // If the file doesn't exist, return false
}
//***********************************************************************************************************************************************************************************************
String getValuesToWebSocket_andWebProcessor(u_int function, String processorAsk)  //default processorAsk = "\0"
{
//  updateDatatoWWW();    //update data true to not send after update values zbyt czesto sie uruchamia -przy kazdej zmiennej zapytania
  String toreturn = "{";
  processorAsk.trim();
  for (u_int i = 0; i < sizeof(ASS)/sizeof(ASS[0]); i++)  /////////////////////////////korekta
  {
    String placeholdername = get_PlaceholderName(i);
    if (ASS[i].Value.length()>0)
    {
      if (i>0) toreturn += F(",");
      toreturn += F("\"");
      toreturn += placeholdername;
      toreturn += F("\":");
      toreturn += F("\"");
      String tmpStr = ASS[i].Value;
      tmpStr.replace("\"","'");
      toreturn += tmpStr;
      toreturn += F("\"");
    }
    if (function == ValuesToWSWPforWebProcessor && (placeholdername).indexOf(processorAsk) >= 0 ) return String(ASS[i].Value);
  }
  if (function == ValuesToWSWPinJSON) return (toreturn + "}");
  return "\0";
}
//***********************************************************************************************************************************************************************************************
void handleWebSocketMessage_sensors(String message)
{
  #ifdef enableWebSocket
  if (!message.isEmpty() and !message.isEmpty() and message.indexOf(":",1) != -1)
  {
    String placeholdertmp = message.substring(0,message.indexOf(":",1));
    String valuetmp = message.substring(message.indexOf(":",1)+1);
    placeholdertmp.trim();
    valuetmp.trim();
    sprintf(log_chars,"Received placeholdertmp: %s, valuetmp: %s", placeholdertmp.c_str(), valuetmp.c_str());
    log_message(log_chars);
    for (u_int i = 0; i < sizeof(ASS)/sizeof(ASS[0]); i++)
    {
      if (ASS[i].Value.length()>0)
      {
        String placeholdername = get_PlaceholderName(i);
        //Serial.println("Debug: "+ASS[i].placeholder+", val: "+ASS[i].Value);
        if (placeholdertmp.indexOf(placeholdername) >= 0) {
          sprintf(log_chars,"Received Websocket %s", String(message).c_str());
          log_message(log_chars);
          ASS[i].Value = valuetmp;
//          receivedwebsocketdata = true;
          #if defined(enableMQTT) || defined(ENABLE_INFLUX)
          receivedmqttdata = true;

          #endif
          updateDatatoWWW_received(i);
          notifyClients("{\""+String(placeholdername)+"\":\""+ASS[i].Value+"\"}");
//          notifyClients(getValuesToWebSocket_andWebProcessor(ValuesToWSWPinJSON));
        }
      }
    }
  }
  #endif
}

//***********************************************************************************************************************************************************************************************
const char edit_html[] =("<!DOCTYPE html> <html> <head> <title>%ME_TITLE% Uploader</title> <meta content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=0' name='viewport'> </head> <body> <center> <header> <div class=\"topnav\"> <h1>%ME_TITLE%</h1> </div> <br> <h2>Uploader</h2> </header> <div> <p style=\"text-align: center\">Use this page to upload new files to the ESP.<br/>You can use compressed (deflated) files (files with a .gz extension) to save space and bandwidth.<br/>Existing files will be replaced.</p> <form method=\"post\" enctype=\"multipart/form-data\" style=\"margin: 0px auto 8px auto\" > <input type=\"file\" name=\"Choose file\" accept=\".gz,.html,.ico,.js,.css\"> <input class=\"button\" type=\"submit\" value=\"Upload\" name=\"submit\"> </form> </div> <br><br> %DIR_LIST% </center> </body> </html>");
//***********************************************************************************************************************************************************************************************
void Setup_WebServer()
{
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Server",String(me_lokalizacja));
  DefaultHeaders::Instance().addHeader("Title",String(me_lokalizacja));
  #ifdef enableWebUpdate
  SetupWebUpdate();
  #endif

//  webserver.rewrite("/", "/index.html");
//  webserver.rewrite("/edit.html", "/edit");
//  webserver.serveStatic("/index.html", SPIFFS, "/index.html");//no cache so can change
  webserver.on("/" , HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html; charset=utf-8",  false, web_processor);
  }).setAuthentication("", "");

  if (!SPIFFS.exists("/edit.html")) {
    webserver.rewrite("/", "/edit");
    webserver.on("/edit" , HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html; charset=utf-8",  edit_html, web_processor);
    }).setAuthentication("", "");
  } else
  {
    webserver.on("/edit" , HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/edit.html", "text/html; charset=utf-8",  false, web_processor);
    }).setAuthentication("", "");
  }


  webserver.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam("RESTART"))
      {
        restart();
      }else {
          //message = "No message sent PARAM_roomtempset0";
      }    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Update value");
    response->addHeader("Refresh", "1");
    response->addHeader("Location", "/");
    request->send(response);
  });

  // webserver.on("/edit.html",  HTTP_POST, [&](AsyncWebServerRequest *request) {  // If a POST request is sent to the /edit.html address,
  //         // the request handler is triggered after the upload has finished...
  //     // create the response, add header, and send response
  //     AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/edit.html", "text/plain; charset=utf-8", false, web_processor);
  //     response->addHeader("Connection", "close");
  //     request->send(response);
  //     },
  //   [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data,
  //                 size_t len, bool final) {webhandleFileUpload(request, filename, index, data, len, final);}
  // ).setAuthentication("", "");


  //webserver.serveStatic("/", SPIFFS, "/").setCacheControl("private, max-age=600");


  // Start a HTTP server with a file read handler and an upload handler
 //     log_message((char*)F("Set cache!"));
      // Serve a file with no cache so every tile It's downloaded
 //     webserver.serveStatic("/configuration.json", SPIFFS, "/configuration.json","no-cache, no-store, must-revalidate");
      // Server all other page with long cache so browser chaching they
      // Comment this line for esp8266
 //     webserver.serveStatic("/", SPIFFS, "/","max-age=31536000");



// run handleUpload function when any file is uploaded
//  webserver.addHandler(webhandleFileRead);
//  webserver.addHandler(webhandleFileUpload);
  webserver.onFileUpload(webhandleFileUpload);
  webserver.onNotFound([](AsyncWebServerRequest * request) {        // if someone requests any other file or page, go to function 'handleNotFound'
    log_message((char*)F("On not found"));
    if(!webhandleFileRead(request, request->url())){                        // check if the file exists in the flash memory (SPIFFS), if so, send it
      webhandleNotFound(request);      // and check if the file exists
    }
  });

  webserver.begin();                             // start the HTTP server
#ifdef enableWebSocket
//  notifyClients(getValuesToWebSocket_andWebProcessor(ValuesToWSWPinJSON));
#endif
  log_message((char*)F("HTTP server started."));
}
//***********************************************************************************************************************************************************************************************
void webhandleNotFound(AsyncWebServerRequest *request) {
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += request->url();
  message += F("\nMethod: ");
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += request->args();
  message += F("\n");

  for (uint8_t i = 0; i < request->args(); i++) {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }
  request->send(404, "text/plain", message);
}
//***********************************************************************************************************************************************************************************************
void webhandleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){ // upload a new file to the SPIFFS
    struct uploadRequest {
      uploadRequest* next;
      AsyncWebServerRequest *request;
      File uploadFile;
      uint32_t fileSize;
      uploadRequest(){next = NULL; request = NULL; fileSize = 0;}
    };
    static uploadRequest uploadRequestHead;
    uploadRequest* thisUploadRequest = NULL;

    if( ! index){
      String toFile = filename;
      if(request->hasParam("dir", true)) {
        AsyncWebParameter* p = request->getParam("dir", true);
        sprintf(log_chars,"dir param: %s", String(p->value()).c_str());
        log_message(log_chars);
        toFile = p->value();
        if(!toFile.endsWith("/")) toFile += "/";
        toFile += filename;
      }
      if(!toFile.startsWith("/")) toFile = "/" + toFile ;

      if(SPIFFS.exists(toFile)) SPIFFS.remove(toFile);
      thisUploadRequest = new uploadRequest;
      thisUploadRequest->request = request;
      thisUploadRequest->next = uploadRequestHead.next;
      uploadRequestHead.next = thisUploadRequest;
      thisUploadRequest->uploadFile = SPIFFS.open(toFile, "w");
      sprintf(log_chars,"Upload: START, filename: %s", String(toFile).c_str());
      log_message(log_chars);
    }
    else {
      thisUploadRequest = uploadRequestHead.next;
      while(thisUploadRequest->request != request) thisUploadRequest = thisUploadRequest->next;
    }

    if(thisUploadRequest->uploadFile){
      for(size_t i=0; i<len; i++){
        thisUploadRequest->uploadFile.write(data[i]);
      }
      thisUploadRequest->fileSize += len;
    }

    if(final){
      thisUploadRequest->uploadFile.close();
      sprintf(log_chars,"Upload: END, Size: %s", String(formatBytes((size_t)thisUploadRequest->fileSize)).c_str());
      log_message(log_chars);
      uploadRequest* linkUploadRequest = &uploadRequestHead;
      while(linkUploadRequest->next != thisUploadRequest) linkUploadRequest = linkUploadRequest->next;
      linkUploadRequest->next = thisUploadRequest->next;
      delete thisUploadRequest;
      AsyncWebServerResponse *response = request->beginResponse(303, "text/plain", "Please wait ...OK");
      response->addHeader("Refresh", "2");
      response->addHeader("Location", "/success.html");
      request->send(response);
      if ( filename.indexOf("index.htm") >-1 ) restart();
    }
}
//***********************************************************************************************************************************************************************************************
void MainCommonSetup()
{
  Serial.begin(115200);
  Serial.println(F("Starting... MainSetup..."));
  getFreeMemory();
  #ifdef doubleResDet
  // double reset detect from start
  doubleResetDetect();
  #endif

  int ledpin = LED_BUILTIN;
  pinMode(ledpin, OUTPUT);
  #ifdef ESP32
  btStop();   //disable bluetooth
  #endif

  if (loadConfig())
  {
    Serial.println(F("Config loaded:"));
    Serial.println(CONFIGURATION.version);
    Serial.println(CONFIGURATION.ssid);
    Serial.println(CONFIGURATION.pass);
    Serial.println(CONFIGURATION.mqtt_server);
  }
  else
  {
    Serial.println(F("Config not loaded!"));
    #ifdef enableWebSocket
    SPIFFS.begin();
    SPIFFS.format();
    #endif
    SaveConfig(); // overwrite with the default settings
  }

  Setup_WiFi();
  runNumber++;
  #ifdef enableArduinoOTA
  Setup_OTA();
  #endif
  Setup_DNS();
#ifdef enableWebSerial
  WebSerial.begin(&webserver);
  WebSerial.msgCallback(recvMsg);
#endif
  starting = false;                   //we have webserial so we can enable it now
  #ifdef enableWebSocket
  Setup_FileSystem();
  Setup_WebSocket();
  Setup_WebServer();    //new www based on spiff files
  updateDatatoWWW();
  #else
  starthttpserver();    //old www
  #endif
  Setup_Mqtt();
  Setup_Influx();
  uptime = millis();
  #ifdef enableMESHNETWORK
  Setup_MeshWiFi();
  #endif
}
//***********************************************************************************************************************************************************************************************
void MainCommonLoop()
{
  #include "configmqtttopics.h"
  #ifdef doubleResDet
  drd->loop();
  #endif
  log_message((char*)F("Main Common Loop..."));
  #ifdef enableArduinoOTA
  // Handle OTA first.
  ArduinoOTA.handle();
  #endif
  check_wifi();
  #ifdef enableMESHNETWORK
  MeshWiFi.update();
  #endif
  #ifdef enableMQTT
  mqttclient.loop();
  #endif
  #ifdef enableWebSocket
  WebSocket.cleanupClients();
  #endif
  // #ifdef enableWebSocket       //Async libraries don't need it at loop
  // WebSocket.loop();                           // constantly check for websocket events
  // webserver.handleClient();                      // run the server
  // #endif
  if (Serial.available() > 0)
  {
    u_int8_t *data;
    String test = Serial.readString();
    data = (u_int8_t *)test.c_str();
    recvMsg(data, (size_t)(test.length()-1));
    //free(data);
  }

  if ((millis() - lastloopRunTime) > LOOP_WAITTIME and WiFi.status() == WL_CONNECTED)
  {
    lastloopRunTime = millis();
    log_message((char*)F("Timing start getValuesToWebSocket_andWebProcessor datawww"));
    updateDatatoWWW();
    log_message((char*)F("Timing start getValuesToWebSocket_andWebProcessor datawww (updateDatatoWWW)"));
    notifyClients(getValuesToWebSocket_andWebProcessor(ValuesToWSWPinJSON));
    log_message((char*)F("Timing start getValuesToWebSocket_andWebProcessor datawww (notifyClients)"));
    // check mqtt
  #ifdef enableMQTT
    if ((WiFi.isConnected()) && (!mqttclient.connected()))
    {
      log_message((char *)F("Lost MQTT connection! Trying Reconnect"));
      mqttclient.disconnect();
      mqttReconnect();
    }
  #endif
  #ifdef ENABLE_INFLUX
    if (InfluxClient.validateConnection())
    {
      sprintf(log_chars, "Connected to InfluxDB: %s", String(InfluxClient.getServerUrl()).c_str());
      log_message(log_chars,0);
    }
    else
    {
      sprintf(log_chars, "InfluxDB connection failed: %s", String(InfluxClient.getLastErrorMessage()).c_str());
      log_message(log_chars,0);
    }
  #endif
    // log stats
    //    #include "configmqtttopics.h"
    String message = F("stats: Uptime: ");
    message += uptimedana();
    message += F(" ## Free memory: ");
    message += String(getFreeMemory());
    message += F("% ");
    message += formatBytes(ESP.getFreeHeap());
    message += F(" bytes ## Wifi: ");
    message += String(getWifiQuality());
    #ifdef enableMQTT
    message += F("% ## Mqtt reconnects: ");
    message += String(mqttReconnects);
    #endif
    log_message((char *)message.c_str());


    String stats = F("{\"CRT\":");
    stats += String(runNumber);
    stats += F(",\"rssi\":");
    stats += String(WiFi.RSSI());
    stats += F(",\"uptime\":");
    stats += String(millis());
    stats += F(",\"version\":");
    stats += String(version);
    stats += F(",\"voltage\":");
    stats += 0;
    stats += F(",\"free memory\":");
    stats += String(getFreeMemory());
    stats += F(",\"ESP_cyclecount\":");
    stats += String(ESP.getCycleCount());
    stats += F(",\"wifi\":");
    stats += String(getWifiQuality());
    #ifdef enableMQTT
    stats += F(",\"mqttReconnects\":");
    stats += mqttReconnects;
    #endif
    stats += F("}");
    #ifdef enableMQTT
    mqttclient.publish(String(STATS_TOPIC).c_str(), stats.c_str(), mqtt_Retain);


    // get new data
    //  if (!heishamonSettings.listenonly) send_panasonic_query();

    // Make sure the LWT is set to Online, even if the broker have marked it dead.
    mqttclient.publish(String(WILL_TOPIC).c_str(), "Online");
    //updateDatatoWWW();  //send after sync struct with vars
    #endif
    if (WiFi.isConnected())
    {
      //      MDNS.announce();
    }
  }

  if (((millis() - lastUpdatemqtt) > mqttUpdateInterval_ms) or lastUpdatemqtt == 0 or receivedmqttdata == true) // recived data ronbi co 800ms -wylacze ten sttus dla odebrania news
  {
    sprintf(log_chars, "mqtt+influxDB lastUpdatemqtt: %s receivedmqttdata: %s mqttUpdateInterval_ms: %s", String(lastUpdatemqtt).c_str(), String(receivedmqttdata).c_str(), String(mqttUpdateInterval_ms).c_str());
    log_message(log_chars,0);
    receivedmqttdata = false;
    lastUpdatemqtt = millis();
#ifdef ENABLE_INFLUX
    updateInfluxDB(); // i have on same server mqtt and influx so when mqtt is down influx probably also ;(  This   if (InfluxClient.isConnected())  doesn't work forme 202205
#endif
    #ifdef enableMQTT
    updateMQTTData();
    #endif
  }

  if ((millis() % 600) < 100) digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

//***********************************************************************************************************************************************************************************************
void Setup_Mqtt()
{
  #ifdef enableMQTT
  mqttclient.setBufferSize(2048);
  mqttclient.setSocketTimeout(3);
  mqttclient.setKeepAlive(5); // fast timeout, any slower will block the main loop too long
  mqttclient.setServer(mqtt_server, mqtt_port);
  mqttclient.setCallback(mqtt_callback);
  mqttReconnect();
  #endif
}
//***********************************************************************************************************************************************************************************************
void mqttReconnect()
{
  log_message((char*)("!!!!mqttReconnect"));
  #ifdef enableMQTT
  mqttReconnects ++;
  mqttclient.disconnect();
  // Loop until we're reconnected
  while (!mqttclient.connected() and mqtt_offline_retrycount < mqtt_offline_retries)
  {
    //log_message((char*)F("Attempting MQTT connection..."));
    const char *clientId = String(BASE_TOPIC).c_str();
    yield();

    if (mqttclient.connect(clientId, mqtt_user, mqtt_password))
    {
      log_message((char*)F("Success MQTT reconnection..."));
      mqtt_offline_retrycount = 0;
      mqttReconnect_subscribe_list();
    }
    else
    {
      sprintf(log_chars,"MQTT reconnect failed, rc state=%s, try again in 0 seconds.", String(mqttclient.state()).c_str());
      log_message(log_chars);
      mqtt_offline_retrycount++;
      // Wait 5 seconds before retrying
      //delay(2000);
    }
  }
  if (mqtt_offline_retrycount == mqtt_offline_retries)
  {
    mqtt_offline_retrycount = 0;


 //   WiFi.disconnect();
  }
  #endif
}
//***********************************************************************************************************************************************************************************************
String formatBytes(size_t bytes) { // convert sizes in bytes to KB and MB
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "kB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
  return "B";
}
//***********************************************************************************************************************************************************************************************
String webgetContentType(String filename){
  if(filename=="download") return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html; charset=utf-8";
  else if(filename.endsWith(".html")) return "text/html; charset=utf-8";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain; charset=utf-8";
}
//***********************************************************************************************************************************************************************************************

/*
 *  check_wifi will process wifi reconnecting managing
 */
void check_wifi()
{
  wdt_reset();
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {//|| (!WiFi.localIP()))  {
    /*
     *  if we are not connected to an AP
     *  we must be in softAP so respond to DNS
     */
    dnsServer.processNextRequest();

    /* we need to stop reconnecting to a configured wifi network if there is a hotspot user connected
     *  also, do not disconnect if wifi network scan is active
     */
    // if ((ssid[0] != '\0') && (WiFi.status() != WL_DISCONNECTED) && (WiFi.scanComplete() != -1) ){ // && (WiFi.softAPgetStationNum() > 0))  {
    //   log_message((char *)"WiFi lost, but softAP station connecting, so stop trying to connect to configured ssid...");
    //   WiFi.disconnect();
    // }

    /*  only start this routine if timeout on
     *  reconnecting to AP and SSID is set
     */
  //  WiFi.disconnect();
    #ifdef enableMQTT
 ///   mqttclient.disconnect();
    #endif

    if ((ssid[0] != '\0') && ((unsigned long)(millis() - lastWifiRetryTimer) > WIFIRETRYTIMER ) )  {
      lastWifiRetryTimer = millis();
      if (WiFi.softAPSSID() == "") {
        log_message((char *)"WiFi lost, starting setup hotspot...");
//        WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255, 255, 255, 0));
//        WiFi.softAP(String(me_lokalizacja).c_str());
      }

      if ((WiFi.status() == WL_DISCONNECTED) || WiFi.status() == 0 ) {//} && (WiFi.softAPgetStationNum() == 0 )) {
        sprintf(log_chars,"Retrying configured WiFi, ... SSID: %s, pss: %s", String(ssid).c_str(), String(pass).c_str());
        log_message((log_chars));
        if (pass[0] == '\0') {
          WiFi.begin(ssid);
        } else {
          WiFi.begin(ssid, pass);
        }
//        Setup_OTA();
//        Setup_DNS();

//        Setup_Mqtt();

      } else {
        log_message((char *)"Reconnecting to WiFi failed. Waiting a few seconds before trying again.");
        WiFi.disconnect();
      }
    }
  } else { //WiFi connected
    if (WiFi.softAPSSID() != "") {
//      log_message((char *)"WiFi (re)connected, shutting down hotspot...");
//      WiFi.softAPdisconnect(true);
//      MDNS.notifyAPChange();
      #ifndef ESP32
//      experimental::ESP8266WiFiGratuitous::stationKeepAliveSetIntervalMs(5000); //necessary for some users with bad wifi routers
      #endif
    }

    if (firstConnectSinceBoot) { // this should start only when softap is down or else it will not work properly so run after the routine to disable softap
      firstConnectSinceBoot = false;
      lastmqtt_reconnect = 0; //initiate mqtt connection asap
      Setup_OTA();
      Setup_DNS();

      if (ssid[0] == '\0') {
        log_message((char *)"WiFi connected without SSID and password in settings. Must come from persistent memory. Storing in settings.");
        WiFi.SSID().toCharArray(ssid, 40);
        WiFi.psk().toCharArray(pass, 40);
        SaveConfig(); //save to config file
      }
    }

    /*
       always update if wifi is working so next time on ssid failure
       it only starts the routine above after this timeout
    */
    lastWifiRetryTimer = millis();
    sprintf(log_chars,"Adres after reconnect IP: %s",WiFi.localIP().toString().c_str() );
    log_message(log_chars);

    // Allow MDNS processing
//    MDNS.update();
  }
  //Serial.println("WiFi Status: "+String(WiFi.status()));

}

//***********************************************************************************************************************************************************************************************

#ifdef enableWebUpdate
const char htmlup[] PROGMEM = R"rawliteral(
  <form method='POST' action='/doUpdate' enctype='multipart/form-data'><input type='file' name='update' accept=".bin,.bin.gz"><input type='submit' value='Update'></form>)rawliteral";
#endif
//***********************************************************************************************************************************************************************************************
void SetupWebUpdate()
{
  #ifdef enableWebUpdate
  webserver.on("/update", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/html; charset=utf-8", htmlup);
    }).setAuthentication("", "");
  webserver.on("/doUpdate", HTTP_POST,
      [&](AsyncWebServerRequest *request) {
      // the request handler is triggered after the upload has finished...
      // create the response, add header, and send response
      AsyncWebServerResponse *response = request->beginResponse((Update.hasError())?500:200, "text/plain; charset=utf-8", (Update.hasError())?"FAIL":"OK");
      response->addHeader("Connection", "close");
      response->addHeader("Access-Control-Allow-Origin", "*");
      request->send(response);
      },
    [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data,
                  size_t len, bool final) {handleDoUpdate(request, filename, index, data, len, final);}
  ).setAuthentication("", "");
  #endif
}
//***********************************************************************************************************************************************************************************************
void handleUpdate(AsyncWebServerRequest *request) {
  #ifdef enableWebSocket
   // Disable client connections
   WebSocket.enable(false);
   // Advertise connected clients what's going on
   notifyClients("Web OTA Update Started");
   // Close them
   WebSocket.closeAll();
  #endif
  #ifdef enableWebUpdate
  request->send(200, "text/html", htmlup);
  #endif
}

void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  #ifdef enableWebUpdate
//#define U_FLASH   0  //copied from ESP8266 library dla ESP32 sprawdzic
//#define U_LittleFS  100
//#define U_AUTH    200
  if (!index) {
    log_message((char*)F("Update starts"));
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
    response->addHeader("Refresh", "15");
    response->addHeader("Location", "/");
    request->send(response);
    content_len = request->contentLength();
    // if filename includes LittleFS, update the LittleFS partition
    int cmd = U_FLASH; //(filename.indexOf("LittleFS") > -1) ? U_LittleFS : U_FLASH;
//    Update.runAsync(true);
    if (!Update.begin(content_len, cmd)) {
      Update.printError(Serial);
    }
  }
  if (Update.write(data, len) != len) {
    Update.printError(Serial);
//#ifdef ESP8266
  } else {
    if ((Update.progress() * 100) / Update.size() % 5 == 0) {
    sprintf(log_chars,"Progress: %d%%", (Update.progress() * 100) / Update.size());
    log_message(log_chars);
    }
//#endif
  }
  if (final) {
    if (!Update.end(true)) {
      Update.printError(Serial);
    } else {
      log_message((char*)F("Update complete"));
      Serial.flush();
      AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
      response->addHeader("Refresh", "20");
      response->addHeader("Location", "/");
      request->send(response);
      delay(100);
      ESP.restart();
    }
  }
  #endif
}
//***********************************************************************************************************************************************************************************************
 void printProgress(size_t prg, size_t sz) {
  #ifdef enableWebUpdate
   if ((Update.progress() * 100) / Update.size() % 5 == 0) {
   sprintf(log_chars,"Progress: %d%%", (prg * 100) / content_len);
   log_message(log_chars);
   }
   #endif
 }
//***********************************************************************************************************************************************************************************************
 String PrintHex8(const uint8_t *data, char separator, uint8_t length) // prints 8-bit data in hex , uint8_t length
{
  uint8_t lensep = sizeof(separator);
  int dod = 0;
  if (separator == 0x0) {
    lensep = 0;
    dod = 1;
  }
  if (lensep > 1) dod = 1 - lensep;
  char tmp[length * (2 + lensep) + dod - lensep];
  byte first;
  byte second;
  for (int i = 0; (i + 0) < length; i++) {
    first = (data[i] >> 4) & 0x0f;
    second = data[i] & 0x0f;
    // base for converting single digit numbers to ASCII is 48
    // base for 10-16 to become lower-case characters a-f is 87
    // note: difference is 39
    tmp[i * (2 + lensep)] = first + 48;
    tmp[i * (2 + lensep) + 1] = second + 48;
    if ((i) < length and (i) + 1 != length) tmp[i * (2 + lensep) + 2] = separator;
    if (first > 9) tmp[i * (2 + lensep)] += 39;
    if (second > 9) tmp[i * (2 + lensep) + 1] += 39;

  }
  tmp[length * (2 + lensep) + 0 - lensep] = 0;
// #ifdef debug
//   Serial.print(F("MAC Addr: "));
//   Serial.println(tmp);
// #endif
  //     debugA("%s",tmp);
  return tmp;
}
//***********************************************************************************************************************************************************************************************
