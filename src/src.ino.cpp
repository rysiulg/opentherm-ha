# 1 "C:\\Users\\Rysza\\AppData\\Local\\Temp\\tmp_r5tgt1q"
#include <Arduino.h>
# 1 "//nas/soft/EPROM/!Projekty/opentherm-ha/src/src.ino"
# 12 "//nas/soft/EPROM/!Projekty/opentherm-ha/src/src.ino"
const float InitTemp = 255;

#include "declarations.h"
#include "main.h"
float pid(float sp, float pv, float pv_last, float &ierr, float dt);
String Boiler_Mode();
void setup();
void loop();
#line 23 "//nas/soft/EPROM/!Projekty/opentherm-ha/src/src.ino"
float pid(float sp, float pv, float pv_last, float &ierr, float dt)
{
  float KP = 10;
  float KI = 0.02;


  float error = sp - pv;

  ierr = ierr + KI * error * dt;



  float P = KP * error;
  float I = ierr;
  float op = P + I;

  if ((op < opcolo) || (op > opcohi))
  {
    I = I - KI * error * dt;


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


  sensors.begin();
  sensors.requestTemperatures();
  sensors.setWaitForConversion(false);
  dallasTemp = sensors.getTempCByIndex(0);
  if (check_isValidTemp(dallasTemp)) dallasTemp = InitTemp;

#ifdef debug
  log_message((char*)F("end setup...."));

#endif
}


void loop()
{
  MainCommonLoop();
  unsigned long now = millis();


  if ((now - lastUpdate) > statusUpdateInterval_ms)
  {

    lastUpdate = now;
    opentherm_update_data();
  }

  if (status_FlameOn) {
    unsigned long long nowtime = millis();
    float boiler_power = 0;
    if (retTemp < boiler_50_30_ret) boiler_power = boiler_50_30; else boiler_power = boiler_80_60;
    double boilerpower = boiler_power * (flame_level / 100);
    double time_to_hour = (nowtime - start_flame_time) / (double(hour_s) * 1000);
    flame_time_total += (nowtime - start_flame_time) ;
    if (status_WaterActive) {
      flame_time_waterTotal += (nowtime - start_flame_time) ;
      flame_used_power_waterTotal += boilerpower * time_to_hour / 1000;
    }
    if (status_CHActive) {
      flame_time_CHTotal += (nowtime - start_flame_time) ;
      flame_used_power_CHTotal += boilerpower * time_to_hour / 1000;
    }

    start_flame_time = nowtime;
    flame_used_power += boilerpower * time_to_hour / 1000;
    flame_used_power_kwh += boilerpower * time_to_hour / 1000;
    sprintf(log_chars, "nowtime: %s, BoilerPower: %s, time_to_hour: %s, flame_used_power bp*time/1k/1k: %s, Flame_Time_Total: %s", String(nowtime).c_str(), String(boilerpower, 6).c_str(), String(time_to_hour).c_str(), String(flame_used_power, 6).c_str(), uptimedana(flame_time_total, true).c_str());
    log_message(log_chars);
  }


  if (((now - lastNEWSSet) > (temp_NEWS_interval_reduction_time_ms * 2)) and 1 == 0)
  {
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
      CO_PumpWorking = false;
      log_message((char*)F("Force disable CO_PumpWorking flag -after 10times execute every 30minutes lowered temp NEWS"));
      temp_NEWS_count = 0;
    }

    log_message((char*)F("Korekta temperatury NEWS z braku połaczenia -pomniejszona o 5%"));
  }

  if ((now - lastSaveConfig) > save_config_every)
  {
    lastSaveConfig = now;
    log_message((char*)F("Saving config to EEPROM memory..."));
    SaveConfig();



  sensors.requestTemperatures();
  sensors.setWaitForConversion(false);
  dallasTemp = sensors.getTempCByIndex(0);
  if (check_isValidTemp(dallasTemp)) dallasTemp = InitTemp;
  sprintf(log_chars, "Current temperature 18B20: %s °C ", String(dallasTemp).c_str());
  log_message(log_chars);

  }

}