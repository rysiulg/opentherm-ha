/*
  Topic structure is built with get/set mechanizm for
  compatibility with home assistant and to allow external
  device control
  _GET_ topics are used to publish current thermostat state
  _SET_ topics are used to control the thermostat

  Works also offline.

*/

// v.1.19 added load/save config to eeprom
// v1.20 added counting used power
// v1.21 added influxdb integration

#define PL_lang       //there are some translations.... -but now is not actual
//#define loctmp "_TMP"     //appender for temporary name in influx and mqtt to not make trash in testing



//#define debug
//#define debugweb

#define enableDebug2Serial    //send log_message to websocket
#define enableWebSocketlog  //send log to websocket AND enableWebSocket MUST BE ENABLED -this replaces me webserial -files on web to build ... to get it
#define enableArduinoOTA
#define enableWebSocket      //ESPAsyncWebServer
#define doubleResDet        //  khoih-prog/ESP_DoubleResetDetector @ ^1.3.1)  check if double reset occurs -if yes stay on OTA ready
#define ENABLE_INFLUX        // tobiasschuerg/ESP8266 Influxdb @ ^3.12.0   if defined sending to influx database is performed at time when mqtt messages is send  -about 130kB of code
#define enableInfluxClient     // works as client -uses include InfluxdbClient.h not Influxdb.h
#define enableMQTTAsync     //Async MQTT   ottowinter/AsyncMqttClient-esphome @ ^0.8.6  -implements also reconnects based by wifi
//#define enableMQTT        //knolleary/PubSubClient@^2.8  --problem with connected status ---
#define enableWebUpdate   //not implemented -not working well
//#define enableMESHNETWORK
//#define enableWebSerial     //not fully implemented and abandoned




#if defined enableWebSocketlog && not defined enableWebSocket
#undef enableWebSocketlog
#endif


#define MFG "MARM.pl Sp. z o.o."
#ifndef loctmp
#define loctmp "\0"
#endif
#define hour_s 60*60          //hour in second

#include "sensivity-config-data.h" //it have definitions of sensivity data
#include "config-translate.h" //definitions polish/english translate
#ifndef SSID_Name
#define SSID_Name "SSID_Name"
#endif
#ifndef SSID_PAssword
#define SSID_PAssword "SSID_PAssword"
#endif
#ifndef MQTT_username
#define MQTT_username "MQTT_username"
#endif
#ifndef MQTT_Password_data
#define MQTT_Password_data "MQTT_Password_data"
#endif
#ifndef MQTT_port_No
#define MQTT_port_No 1883     //default mqtt port
#endif
#ifndef MQTT_servername
#define MQTT_servername "MQTT_servername"     //default mqtt port
#endif

#ifdef ENABLE_INFLUX
#ifndef INFLUXDB_URL
#define INFLUXDB_URL "http://localhost:8086"
#endif
// InfluxDB 2 server or cloud API authentication token (Use: InfluxDB UI -> Load Data -> Tokens -> <select token>) but I use only version 1 as default in HomeAssistant
#ifndef INFLUXDB_DB_NAME
#define INFLUXDB_DB_NAME "test"
#endif
#ifndef INFLUXDB_USER
#define INFLUXDB_USER "test"
#endif
#ifndef INFLUXDB_PASSWORD
#define INFLUXDB_PASSWORD "test"
#endif
#define InfluxMeasurments "MARMpl_Measurments"
#endif



// Your WiFi credentials.
// Set password to "" for open networks.
#define sensitive_size 32
#define sensitive_sizeS "32" //length of ssid,passwd and mqtt needed as string for version check -important 2 letters



#include "declarations_for_websync.h"
