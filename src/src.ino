/*************************************************************
  This example runs directly on ESP8266 chip.

  Please be sure to select the right ESP8266 module
  in the Tools -> Board -> WeMos D1 Mini

  Adjust settings in Config.h before run
 *************************************************************/

//#include <Arduino.h>

const float InitTemp = 255;
// ESP8266WebServer server(80);
#include "declarations.h"
#include "main.h"






// op = pid(sp, t, t_last, ierr, dt);
float pid(float sp, float pv, float pv_last, float &ierr, float dt)
{
  float KP = 10;
  float KI = 0.02;

  // calculate the error
  float error = sp - pv;
  // calculate the integral error
  ierr = ierr + KI * error * dt;
  // calculate the measurement derivative
  // float dpv = (pv - pv_last) / dt;
  // calculate the PID output
  float P = KP * error; // proportional contribution
  float I = ierr;       // integral contribution
  float op = P + I;
  // implement anti-reset windup
  if ((op < opcolo) || (op > opcohi))
  {
    I = I - KI * error * dt;
    // clip output
    
    op = max(opcolo, min(opcohi, op));
  }
  ierr = I;
  sprintf(log_chars, "sp=%s, pv=%s, dt=%s, op=%s, P=%s, I=%s, tNEWS=%s", String(sp).c_str(), String(pv).c_str(), String(dt).c_str(), String(op).c_str(), String(P).c_str(), String(I).c_str(), String(temp_NEWS).c_str());
  log_message(log_chars);
  return op;
}


String Boiler_Mode()
{
  if (automodeCO and heatingEnabled)
  { return "auto";
  } else if (heatingEnabled) {
    return "heat";
  } else
  {
    return "off";
  }
}

void setup()
{

  MainCommonSetup();

  ot.begin(handleInterrupt);

  // Init DS18B20 sensor
  sensors.begin();
  sensors.requestTemperatures();
  sensors.setWaitForConversion(false); // switch to async mode
  dallasTemp = sensors.getTempCByIndex(0);
  if (check_isValidTemp(dallasTemp)) dallasTemp = InitTemp;
  //  lastTempSet = -extTempTimeout_ms;
#ifdef debug
  log_message((char*)F("end setup...."));
  //    SaveConfig();
#endif
}


void loop()
{
  MainCommonLoop();
  unsigned long now = millis(); // TO AVOID compare -2>10000 which is true ??? why?
  // check mqtt is available and connected in other case check values in api.

  if ((now - lastUpdate) > statusUpdateInterval_ms)
  {

    lastUpdate = now;
    opentherm_update_data(); // According OpenTherm Specification from Ihnor Melryk Master requires max 1s interval communication -przy okazji wg czasu update mqtt zrobie odczyt dallas
  }

  if (status_FlameOn) {
    unsigned long nowtime = millis();
    float boiler_power = 0;
    if (retTemp < boiler_50_30_ret) boiler_power = boiler_50_30; else boiler_power = boiler_80_60;
    double boilerpower = boiler_power * (flame_level / 100); //kW
    double time_to_hour = (nowtime - start_flame_time) / (double(hour_s) * 1000);
    flame_time_total += (nowtime - start_flame_time) / 1000;
    if (status_WaterActive) {
      flame_time_waterTotal += (nowtime - start_flame_time) / 1000;
      flame_used_power_waterTotal += boilerpower * time_to_hour / 1000;
    }
    if (status_CHActive) {
      flame_time_CHTotal += (nowtime - start_flame_time) / 1000;
      flame_used_power_CHTotal += boilerpower * time_to_hour / 1000;
    }

    start_flame_time = 0;
    flame_used_power += boilerpower * time_to_hour / 1000;
    flame_used_power_kwh += boilerpower * time_to_hour / 1000;
    sprintf(log_chars, "nowtime: %s, BoilerPower: %s, time_to_hour: %s, flame_used_power bp*time/1k/1k: %s, Flame_Time_Total: %s", String(nowtime).c_str(), String(boilerpower, 6).c_str(), String(time_to_hour).c_str(), String(flame_used_power, 6).c_str(), uptimedana(flame_time_total, true).c_str());
    log_message(log_chars);
  }

  //#define abs(x) ((x)>0?(x):-(x))
  if (((now - lastNEWSSet) > (temp_NEWS_interval_reduction_time_ms * 2)) and 1 == 0) //disable for now
  { // at every 0,5hour lower temp NEWS when no communication why -2>1800000 is true ???
    sprintf(log_chars, "nowtime: %s, lastNEWSSet: %s, temp_NEWS_interval_reduction_time_ms: %s", String(now).c_str(), String(lastNEWSSet, 6).c_str(), String(temp_NEWS_interval_reduction_time_ms).c_str());
    log_message(log_chars);
    lastNEWSSet = now;
    temp_NEWS_count++;
    if (temp_NEWS > cutOffTemp)
    {
      temp_NEWS = temp_NEWS - temp_NEWS * 0.05;
      sprintf(log_chars, "Lowering by 5%% temp_NEWS (no communication) -after 10times execute every 30minutes lowered temp NEWS from: %s, to: %s", String(temp_NEWS).c_str(), String(temp_NEWS - temp_NEWS * 0.05).c_str());
      log_message(log_chars);
    }
    else
    {
      temp_NEWS = cutOffTemp;
    }
    if (temp_NEWS_count > 10)
    {
      CO_PumpWorking = false; // assume that we loose mqtt connection to other system where is co pump controlled -so after 10 times lowered NEWS temp by 5% we also disable CO_Pump_Working to allow heat by this heater -default it counts 5hours no communication
      log_message((char*)F("Force disable CO_PumpWorking flag -after 10times execute every 30minutes lowered temp NEWS"));
      temp_NEWS_count = 0;
    }
    // dobre miejsce do try get data by http api
    log_message((char*)F("Korekta temperatury NEWS z braku połaczenia -pomniejszona o 5%"));
  }

  if ((now - lastSaveConfig) > save_config_every)
  {
    lastSaveConfig = now;
    log_message((char*)F("Saving config to EEPROM memory..."));
    SaveConfig(); // According OpenTherm Specification from Ihnor Melryk Master requires max 1s interval communication -przy okazji wg czasu update mqtt zrobie odczyt dallas
    // String tempSource = (millis() - lastTempSet > extTempTimeout_ms)
  //                         ? "(internal sensor)"
  //                         : "(external sensor)";
  sensors.requestTemperatures();
  sensors.setWaitForConversion(false); // switch to async mode
  dallasTemp = sensors.getTempCByIndex(0);
  if (check_isValidTemp(dallasTemp)) dallasTemp = InitTemp;
  sprintf(log_chars, "Current temperature 18B20: %s °C ", String(dallasTemp).c_str()); //, tempSource.c_str());
  log_message(log_chars);

  }

}
