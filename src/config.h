/*
  Topic structure is built with get/set mechanizm for
  compatibility with home assistant and to allow external
  device control
  _GET_ topics are used to publish current thermostat state
  _SET_ topics are used to control the thermostat

  Works also offline.

*/

// v.1.19 added load/save config to eeprom

//#define debug
#define debug1
#define me_version "1.19g"
#define me_lokalizacja "BOILER_mqqt_MARM"
#define ATOMIC_FS_UPDATE
#define MFG "MARM.pl Sp. z o.o."
#define stopka MFG " 04-2022"
#define wwwport 80
#define PL_lang

#define hour 60*60
#define boiler_rated_kWh 24

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

// Master OpenTherm Shield pins configuration
const int OT_IN_PIN = D1;  // for Arduino, 4 for ESP8266 (D2), 21 for ESP32
const int OT_OUT_PIN = D2; // for Arduino, 5 for ESP8266 (D1), 22 for ESP32

// Temperature sensor pin
const int ROOM_TEMP_SENSOR_PIN = D5; // 0; //for Arduino, 14 for ESP8266 (D5), 18 for ESP32

/*
   current temperature topics
   if setter is used - thermostat works with external values, bypassing built-in sensor
   if no values on setter for more than 1 minute - thermostat falls back to built-in sensor
*/
const String deviceid = "\"dev\":{\"ids\":\"OpenTherm_GAS\",\"name\":\"OpenTherm GAS-CO\",\"sw\":\"" + String(me_version) + "\",\"mdl\": \"ESP8266_GASBoiler\",\"\mf\":\"" + String(MFG) + "\"}";

const String BASE_TOPIC = "opentherm-thermostat";
const String BASE_HA_TOPIC = "homeassistant";
const String ROOM_TEMP = "current_remote";
const int mqtt_Retain = 1;
const String QOS = "0";
const String OT = "ot_";
const String BOILER = "boiler";
const String HOT_WATER = "domestic_hot_water";
const String ROOM_OTHERS = "room_other";
const String TEMPERATURE = "_temperature";
const String BOILER_TEMPERATURE = BOILER + TEMPERATURE;
// const String BOILER_MOD = BOILER+"-mode";   //tryb pracy
const String BOILER_TEMPERATURE_RET = BOILER + TEMPERATURE + "_return";
const String BOILER_TEMPERATURE_SETPOINT = BOILER + TEMPERATURE + "_setpoint";
const String BOILER_CH_STATE = BOILER + "_ch_state";
const String BOILER_SOFTWARE_CH_STATE_MODE = BOILER + "_software_ch_state_and_mode";
const String FLAME_STATE = "flame_state";
const String FLAME_LEVEL = "flame_level";
const String TEMP_CUTOFF = "temp_cutoff";
const String FLAME_W = "flame_used_energy";

const String HOT_WATER_TEMPERATURE = HOT_WATER + TEMPERATURE;
const String HOT_WATER_TEMPERATURE_SETPOINT = HOT_WATER + TEMPERATURE + "_setpoint";
const String HOT_WATER_CH_STATE = HOT_WATER + "_dhw_state";
const String HOT_WATER_SOFTWARE_CH_STATE = HOT_WATER + "_software_dhw_state";

const String ROOM_OTHERS_TEMPERATURE = ROOM_TEMP + TEMPERATURE;
const String ROOM_OTHERS_TEMPERATURE_SETPOINT = ROOM_TEMP + TEMPERATURE + "_setpoint";
const String ROOM_OTHERS_PRESSURE = ROOM_OTHERS + "_pressure";

const String BOILER_TOPIC = BASE_TOPIC + "/" + BOILER + "/attributes";
const String HOT_WATER_TOPIC = BASE_TOPIC + "/" + HOT_WATER + "/attributes";
const String ROOM_OTHERS_TOPIC = BASE_TOPIC + "/" + ROOM_OTHERS + "/attributes";

const String ROOM_TEMP_SET_TOPIC = BASE_TOPIC + "/SET/" + ROOM_OTHERS_TEMPERATURE_SETPOINT + "/set"; // t
const String TEMP_SETPOINT_SET_TOPIC = BASE_TOPIC + "/SET/" + BOILER_TEMPERATURE_SETPOINT + "/set";  // sp
const String TEMP_CUTOFF_SET_TOPIC = BASE_TOPIC + "/SET/" + TEMP_CUTOFF + "/set";                    // cutOffTemp
const String STATE_DHW_SET_TOPIC = BASE_TOPIC + "/SET/" + HOT_WATER_SOFTWARE_CH_STATE + "/set";      // enableHotWater
const String MODE_SET_TOPIC = BASE_TOPIC + "/SET/" + BOILER_SOFTWARE_CH_STATE_MODE + "/set";         // 012 auto, heat, off ch
const String TEMP_DHW_SET_TOPIC = BASE_TOPIC + "/SET/" + HOT_WATER_TEMPERATURE_SETPOINT + "/set";    // dhwTarget
String COPUMP_GET_TOPIC = "COWoda_mqqt_MARM/switch/bcddc2b2c08e/pump2CO/state";                      // temperatura outside avg NEWS
String NEWS_GET_TOPIC = "COWoda_mqqt_MARM/sensor/bcddc2b2c08e/WENS_Outside_Temp_AVG/state";          // pompa CO status

// logs topic
const String DIAGS = "diag";
const String DIAG_TOPIC = BASE_TOPIC + "/" + DIAGS + "/attributes";
const String DIAG_HA_TOPIC = BASE_HA_TOPIC + "/sensor/" + BASE_TOPIC + "/";
const String DIAG_HABS_TOPIC = BASE_HA_TOPIC + "/binary_sensor/" + BASE_TOPIC + "/";

const String LOGS = "log";
const String LOG_GET_TOPIC = BASE_TOPIC + "/" + DIAGS + "/" + LOGS;
const String INTEGRAL_ERROR_GET_TOPIC = DIAGS + "_" + "interr";
const String DIAGS_OTHERS_FAULT = DIAGS + "_" + "fault";
const String DIAGS_OTHERS_DIAG = DIAGS + "_" + "diagnostic";

//Homeassistant Autodiscovery topics
const String BOILER_HA_TOPIC = BASE_HA_TOPIC + "/sensor/" + BASE_TOPIC + "/";              //+"/state"
const String BOILER_HABS_TOPIC = BASE_HA_TOPIC + "/binary_sensor/" + BASE_TOPIC + "/";     //+"/state"
const String BOILER_HACLI_TOPIC = BASE_HA_TOPIC + "/climate/" + BASE_TOPIC + "/" + BOILER; //+"/state"

const String HOT_WATER_HA_TOPIC = BASE_HA_TOPIC + "/sensor/" + BASE_TOPIC + "/";                 //+"/state"
const String HOT_WATER_HABS_TOPIC = BASE_HA_TOPIC + "/binary_sensor/" + BASE_TOPIC + "/";        //+"/state"
const String HOT_WATER_HACLI_TOPIC = BASE_HA_TOPIC + "/climate/" + BASE_TOPIC + "/" + HOT_WATER; //+"/state"

const String ROOM_OTHERS_HA_TOPIC = BASE_HA_TOPIC + "/sensor/" + BASE_TOPIC + "/" + ROOM_OTHERS;     //+"/state"
const String ROOM_OTHERS_HACLI_TOPIC = BASE_HA_TOPIC + "/climate/" + BASE_TOPIC + "/" + ROOM_OTHERS; //+"/state"



// setpoint topic
const String SETPOINT_OVERRIDE = "setpoint-override";
const String SETPOINT_OVERRIDE_SET_TOPIC = BASE_TOPIC + "/" + SETPOINT_OVERRIDE + "/set";     // op_override
const String SETPOINT_OVERRIDE_RESET_TOPIC = BASE_TOPIC + "/" + SETPOINT_OVERRIDE + "/reset"; //
