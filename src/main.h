
#include "Common_symlinkFiles\common_functions.h"



#include <OneWire.h>
#include <DallasTemperature.h>

#include <OpenTherm.h>


#include <WiFiClient.h>






//#include "config.h"



OneWire oneWire(ROOM_TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);
OpenTherm OpenTherm(OT_IN_PIN, OT_OUT_PIN);

#define spOverrideTimeout_ms (180 * 1000)               //used in main

const unsigned long //extTempTimeout_ms = 180 * 1000,
                    statusUpdateInterval_ms = 0.9 * 1000,
            //        ,
                    temp_NEWS_interval_reduction_time_ms = 30 * 60 * 1000, // time to laps between temp_NEWS reduction by 5%
                    mqtt_offline_reconnect_after_ms = 15 * 60 * 1000,      // time when mqtt is offline to wait for next reconnect (15minutes)
                    save_config_every = 15 * 60 * 1000,                    // time saveing config values in eeprom (15minutes)
                    WiFiinterval = 30 * 1000;

// upper and lower bounds on heater level
const float noCommandSpOverride = 32; //heating water temperature for fail mode (no external temp provided) for co


float dhwTarget = 49.5, // domyslna temperatura docelowa wody uzytkowej
      roomtempSet = 20.5,       // set point roomtemp
      roomtemp = 21,     // current temperature
      roomtemp_last = 0, // prior temperature
      ierr = 0,         // integral error
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
      pressure = 0,
      histCWU = 5,             //histereza dla cwu -obnizenie progu aktywacji grzania
      histCO = 0,             //histereza dla co -obnizenie progu aktywacji grzania
      histlo = 0,             //min wartosc histerezy
      histhi = histerezaCOCWU,             //max wartosc histerezy
      temp_Water2 = 0;                  //remote temp lower part



unsigned long ts = 0, new_ts = 0, // timestamp
              lastUpdate = 0,
              lastTempSet = 0,
              lastcutOffTempSet = 0,
              lastNEWSSet = 0,
              lastmqtt_reconnect = 0,
              lastSaveConfig = 0,
              lastroomtempSet = 0,
              WiFipreviousMillis = 0,
              temp_Water2_time = 0;     //for remote temp water
unsigned long long start_flame_time = 0,    //Flame Starts work
                   start_flame_time_fordisplay = 0,  //for display on status card flame time
                   flame_time_total = 0,         //total sum of flametime
                   flame_time_waterTotal = 0,
                   flame_time_CHTotal = 0;

double  flame_used_power_kwh = 0,     //f or count powerkWh between samples (boiler rated*flame power) use integer end when div use % do get decimal value
        flame_used_power = 0,         //for count used energy
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
     status_flame_prv = false,     //previous status of flame before read new val
     status_Cooling =false,
     status_Diagnostic =false,
     ecoMode = true;


float floor1_temp = InitTemp, floor2_temp = InitTemp, floor1_tempset = InitTemp, floor2_tempset = InitTemp;



//main.cpp
void IRAM_ATTR OT_Handle_Interrupt();
float pid(float sp, float pv, float pv_last, float &ierr, float dt);
void opentherm_update_data();
void getTemp();
String Boiler_Mode();
void setup();
void loop();



//webserver_ota
// bool loadConfig();
// bool SaveConfig();
//String addusage_local_values();   //it is on commonfunc defined







//#include "common_functions.h"
#include "load_save_config.h"
#include "mqttinflux.h"
#include "update_data2web.h"
