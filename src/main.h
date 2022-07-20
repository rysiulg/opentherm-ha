#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <OpenTherm.h>
// #ifdef ICACHE_RAM_ATTR
// #undef ICACHE_RAM_ATTR
// #define ICACHE_RAM_ATTR IRAM_ATTR
// #endif



#include <WiFiClient.h>

// for ota




#include "config.h"
#include "configmqtttopics.h"


typedef struct
{
  char version[6]; // place to detect if settings actually are written
  bool heatingEnabled;
  bool enableHotWater;
  bool automodeCO;
  float tempBoilerSet;   //temp boiler set -mainly used in auto mode and for display on www actual temp
  float roomtempSet;              //romtempsetpoint
  float cutOffTemp;
  float op_override;     //boiler tempset on heat mode
  float dhwTarget;       //hot water temp set
  float roomtemp;        //now is static sensor so for while save last value
  float temp_NEWS;
  char ssid[sensitive_size];
  char pass[sensitive_size];
  char mqtt_server[sensitive_size*2];
  char mqtt_user[sensitive_size];
  char mqtt_password[sensitive_size];
  int mqtt_port;
  char COPUMP_GET_TOPIC[255];  //temperatura outside avg NEWS
  char NEWS_GET_TOPIC[255];   //pompa CO status
  char NEWS_GET_TOPIC1[255];   //pompa CO status for 1st temp room sensor
  char NEWS_GET_TOPIC2[255];   //pompa CO status for 2nd temp room sensor
} configuration_type;

// with DEFAULT values!
configuration_type CONFIGURATION;

OneWire oneWire(ROOM_TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);
OpenTherm ot(OT_IN_PIN, OT_OUT_PIN);

#define spOverrideTimeout_ms (180 * 1000)

const unsigned long //extTempTimeout_ms = 180 * 1000,
                    statusUpdateInterval_ms = 0.9 * 1000,
            //        ,
                    temp_NEWS_interval_reduction_time_ms = 30 * 60 * 1000, // time to laps between temp_NEWS reduction by 5%
                    mqtt_offline_reconnect_after_ms = 15 * 60 * 1000,      // time when mqtt is offline to wait for next reconnect (15minutes)
                    save_config_every = 15 * 60 * 1000,                    // time saveing config values in eeprom (15minutes)
                    WiFiinterval = 30 * 1000;

// upper and lower bounds on heater level
const float noCommandSpOverride = 32; //heating water temperature for fail mode (no external temp provided) for co

unsigned int runNumber = 0, // count of restarts
             publishhomeassistantconfig = 4,                               // licznik wykonan petli -strat od 0
             publishhomeassistantconfigdivider = 5;                        // publishhomeassistantconfig % publishhomeassistantconfigdivider -publikacja gdy reszta z dzielenia =0 czyli co te ilosc wykonan petli opoznionej update jest wysylany config

float dhwTarget = 51, // domyslna temperatura docelowa wody uzytkowej
      roomtempSet = 20.5,       // set point roomtemp
      roomtemp = 21,     // current temperature
      roomtemp_last = 0, // prior temperature
      ierr = 25,         // integral error
      dt = 0,            // time between measurements
      //       op = 0, //PID controller output
      retTemp = 0,                         // return temperature
      temp_NEWS = InitTemp,                       // avg temperatura outside -getting this from mqtt topic as averange from 4 sensors North West East South
      cutOffTemp = 2,                      // outside temp setpoint to cutoff heating co. CO heating is disabled if outside temp (temp_NEWS) is above this value
 //     op_override = noCommandSpOverride, // boiler tempset on heat mode
      flame_level = 0,
      tempBoiler = 0,
      tempBoilerSet = noCommandSpOverride, // temp boiler set -mainly used in auto mode and for display on www actual temp
      tempCWU = 0,
      dallasTemp = 0,
      pressure = 0;


String LastboilerResponseError;



unsigned long ts = 0, new_ts = 0, // timestamp
              lastUpdate = 0,
              lastTempSet = 0,
              lastcutOffTempSet = 0,
              lastNEWSSet = 0,
              lastmqtt_reconnect = 0,
              lastSaveConfig = 0,
              lastroomtempSet = 0,
              WiFipreviousMillis = 0,
              start_flame_time = 0,    //Flame Starts work
              flame_time = 0;         //flame time

double  flame_used_power_kwh = 0,     //f or count powerkWh between samples (boiler rated*flame power) use integer end when div use % do get decimal value
        flame_used_power = 0,         //for count used energy
        flame_time_total = 0,         //total sum of flametime
        flame_time_waterTotal = 0,
        flame_time_CHTotal = 0,
        flame_used_power_waterTotal = 0,
        flame_used_power_CHTotal = 0;

bool heatingEnabled = true,
     enableHotWater = true,
     CO_PumpWorking = false, // value from mqtt -if set co heating is disabled -second heating engine is working (in my case Wood/Carbon heater)
     Water_PumpWorking = false, // value from mqtt -if set water heating engine is working (in my case Wood/Carbon heater)
     automodeCO = false,     // tryb automatyczny zalezny od temp zewnetrznej i pracy pompy CO kotla weglowego
     tmanual = false,
     status_Fault = false,
     status_CHActive = false,
     status_WaterActive =false,
     status_FlameOn =false,
     status_Cooling =false,
     status_Diagnostic =false,
     ecoMode = true;


float floor1_temp = InitTemp, floor2_temp = InitTemp, floor1_tempset = InitTemp, floor2_tempset = InitTemp;



void recvMsg(uint8_t *data, size_t len);
void IRAM_ATTR handleInterrupt();
void getTemp();
float pid(float sp, float pv, float pv_last, float &ierr, float dt);
void opentherm_update_data();
String Boiler_Mode();
void updateInfluxDB();
void updateMQTTData();
void mqtt_callback(char *topic, byte *payload, unsigned int length);


void setup();
void loop();

//websrv_ota
void starthttpserver();
String processor(const String var);
String do_stopkawebsite();
bool loadConfig();
void SaveConfig();

void updateDatatoWWW_assignPlaceholdersOnce();
void updateDatatoWWW_received(u_int i);


#include "common_functions.h"
#include "websrv_ota.h"
#include "mqttinflux.h"
#include "other.h"
