#include "config.h"
#ifdef ENABLE_INFLUX
  #include <ESP8266HTTPClient.h>
  #include <InfluxDbClient.h>
#endif
#include <ArduinoJson.h>


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
const String me_version = String(version);
const String  stopka = String(MFG)+" "+version[4]+version[5]+"-"+version[2]+version[3]+"-20"+version[0]+version[1]+" "+version[6]+version[7]+":"+version[8]+version[9];
const String deviceid = "\"dev\":{\"ids\":\""+String(me_lokalizacja)+"\",\"name\":\""+String(me_lokalizacja)+"\",\"sw\":\"" + String(me_version) + "\",\"mdl\": \""+String(me_lokalizacja)+"\",\"mf\":\"" + String(MFG) + "\"}";

const unsigned long extTempTimeout_ms = 180 * 1000,
                    statusUpdateInterval_ms = 0.9 * 1000,
                    spOverrideTimeout_ms = 180 * 1000,
                    mqttUpdateInterval_ms = 16 * 1000,
                    temp_NEWS_interval_reduction_time_ms = 30 * 60 * 1000, // time to laps between temp_NEWS reduction by 5%
                    mqtt_offline_reconnect_after_ms = 15 * 60 * 1000,      // time when mqtt is offline to wait for next reconnect (15minutes)
                    save_config_every = 15 * 60 * 1000,                    // time saveing config values in eeprom (15minutes)
                    WiFiinterval = 30 * 1000;

// upper and lower bounds on heater level
const float ophi = 65,               // upper max heat water
            oplo = 30,               // lower min heat water
            opcohi = 60,             // upper max heat boiler to CO
            opcolo = oplo,           // lower min heat boiler to CO
            cutoffhi = 20,           // upper max cut-off temp above is heating CO disabled -range +-20
            cutofflo = -cutoffhi,    // lower min cut-off temp above is heating CO disabled
            roomtemphi = 30,         // upper max to set of room temperature
            roomtemplo = 15,         // lower min to set of room temperature
            noCommandSpOverride = 32; //heating water temperature for fail mode (no external temp provided) for co

unsigned int runNumber = 0, // count of restarts
             publishhomeassistantconfig = 4,                               // licznik wykonan petli -strat od 0
             publishhomeassistantconfigdivider = 5;                        // publishhomeassistantconfig % publishhomeassistantconfigdivider -publikacja gdy reszta z dzielenia =0 czyli co te ilosc wykonan petli opoznionej update jest wysylany config

float dhwTarget = 51, // domyslna temperatura docelowa wody uzytkowej
      sp = 20.5,       // set point roomtemp
      roomtemp = 21,     // current temperature
      roomtemp_last = 0, // prior temperature
      ierr = 25,         // integral error
      dt = 0,            // time between measurements
      //       op = 0, //PID controller output
      retTemp = 0,                         // return temperature
      temp_NEWS = 0,                       // avg temperatura outside -getting this from mqtt topic as averange from 4 sensors North West East South
      cutOffTemp = 2,                      // outside temp setpoint to cutoff heating co. CO heating is disabled if outside temp (temp_NEWS) is above this value
      op_override = noCommandSpOverride, // boiler tempset on heat mode
      flame_level = 0,
      tempBoiler = 0,
      tempBoilerSet = op_override, // temp boiler set -mainly used in auto mode and for display on www actual temp
      tempCWU = 0,
      pressure = 0;


String LastboilerResponseError;

int temp_NEWS_count = 0,
    mqtt_offline_retrycount = 0,
    mqtt_offline_retries = 10; // retries to mqttconnect before timewait

unsigned long ts = 0, new_ts = 0, // timestamp
              lastUpdate = 0,
              lastUpdatemqtt = 0,
              lastTempSet = 0,
              lastcutOffTempSet = 0,
              lastNEWSSet = 0,
              lastmqtt_reconnect = 0,
              lastSaveConfig = 0,
              lastSpSet = 0,
              WiFipreviousMillis = 0,
              start_flame_time = 0,    //Flame Starts work
              flame_time = 0;         //flame time

double  flame_used_power_kwh = 0;         //for count powerkWh between samples (boiler rated*flame power) use integer end when div use % do get decimal value
double  flame_used_power = 0;             //for count used energy

bool heatingEnabled = true,
     enableHotWater = true,
     CO_PumpWorking = false, // value from mqtt -if set co heating is disabled -second heating engine is working (in my case Wood/Carbon heater)
     automodeCO = false,     // tryb automatyczny zalezny od temp zewnetrznej i pracy pompy CO kotla weglowego
     receivedmqttdata = false,
     tmanual = false,
     status_Fault,
     status_CHActive,
     status_WaterActive,
     status_FlameOn,
     status_Cooling,
     status_Diagnostic;



bool PayloadStatus(String payloadStr, bool state);
bool PayloadtoValidFloatCheck(String payloadStr);
float PayloadtoValidFloat(String payloadStr,bool withtemps_minmax=false, float mintemp=InitTemp,float maxtemp=InitTemp);
void recvMsg(uint8_t *data, size_t len);
void IRAM_ATTR handleInterrupt();
float getTemp();
float pid(float sp, float pv, float pv_last, float &ierr, float dt);
void opentherm_update_data(unsigned long mqttdallas);
void updateData();
String convertPayloadToStr(byte *payload, unsigned int length);
bool isValidNumber(String str);
void callback(char *topic, byte *payload, unsigned int length);
void reconnect();

void setup();
void loop();