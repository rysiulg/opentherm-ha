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

#define debug
//#define debug1
//#define debugweb
#define me_lokalizacja "BOILER_GAZ"

#define MFG "MARM.pl Sp. z o.o."

#define PL_lang

#define hour_s 60*60          //hour in second
#define boiler_rated_kWh 24
#define boiler_50_30 20.7     //boiler technical factory set data at working 50st and return 30st
#define boiler_50_30_ret 31     //boiler technical factory set data at working 50st and return 30st -assume that is that mode if rettemp i under 31degC
#define boiler_80_60 19.5     //boiler technical factory set data at working 80st and return 60st
#define ENABLE_INFLUX        //if defined sending to influx database is performed at time when mqtt messages is send  -about 130kB of code
#define enableMQTT


//#define wdtreset
//#boiler_gas_conversion_to_m3  1

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
char ssid[sensitive_size] = SSID_Name;
char pass[sensitive_size] = SSID_PAssword;

// Your MQTT broker address and credentials
char mqtt_server[sensitive_size*2] = MQTT_servername;
char mqtt_user[sensitive_size] = MQTT_username;
char mqtt_password[sensitive_size] = MQTT_Password_data;
int mqtt_port = MQTT_port_No;
const int mqtt_Retain = 1;

// Master OpenTherm Shield pins configuration
const int OT_IN_PIN = D1;  // for Arduino, 4 for ESP8266 (D2), 21 for ESP32
const int OT_OUT_PIN = D2; // for Arduino, 5 for ESP8266 (D1), 22 for ESP32

// Temperature sensor pin
const int ROOM_TEMP_SENSOR_PIN = D0; // 0; //for Arduino, 14 for ESP8266 (D5), 18 for ESP32

/*
   current temperature topics
   if setter is used - thermostat works with external values, bypassing built-in sensor
   if no values on setter for more than 1 minute - thermostat falls back to built-in sensor
*/

#define InitTemp 85             //temperatura inicjalna gdy brak odczuty
float floor1_temp = InitTemp, floor2_temp = InitTemp, floor1_tempset = InitTemp, floor2_tempset = InitTemp;  //temps from floor1 nad floor2 temp is min of temps and tempset is max set value
