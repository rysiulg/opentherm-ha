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

// This function calculates temperature every second.
void opentherm_update_data()
{
  log_message((char*)F("Update_OpenTherm_data"));
  // Set/Get Boiler Status
  bool enableCooling = false;
  receivedmqttdata = false;
  bool COHeat = false;
  if (ecoMode) opcohi = ecohi; else opcohi = opcohistatic;
  if (tempBoilerSet > opcohi) tempBoilerSet = opcohi;


  getTemp(); //default returns roomtemp (avg) and as global sp=roomtempset (avg), roomtemp is also global var
  float op = tempBoilerSet;
  if (heatingEnabled)
  {
    //  if (!automodeCO) {
    //this will remove tempset avg from floors tempBoilerSet = op_override;  //for no automode
    if (temp_NEWS < cutOffTemp)
      COHeat = true;
    if (CO_PumpWorking)
      COHeat = false;
    if (temp_NEWS > (cutOffTemp + 0.9))
      COHeat = false;
    //  } else
    if (automodeCO) {
      if (roomtemp < (roomtempSet + 0.6) && COHeat) COHeat = true; else COHeat = false;
      //based on roomtemp<roomtemset
      op = 0;
      unsigned long now = millis();
      new_ts = millis();
      dt = (new_ts - ts) / 1000.0;
      ts = new_ts;
      op = pid(roomtempSet, roomtemp, roomtemp_last, ierr, dt);
      if ((now - lastroomtempSet <= spOverrideTimeout_ms))
      {
        op = noCommandSpOverride;
      }
      //    tempBoilerSet = op;
      roomtemp_last = roomtemp;
    }
  }

sprintf(log_chars,"Statusy openth: COHEAT: %s, DHW: %s, automodeCO: %s, temp_NEWS: %s", String(COHeat?"chodzi":"stoi").c_str(), String(enableHotWater?"chodzi":"stoi").c_str(), String(automodeCO?"ON":"stoi").c_str(), String(temp_NEWS).c_str());
log_message(log_chars);
//COHeat = false;

  unsigned long response = ot.setBoilerStatus(COHeat, enableHotWater, enableCooling); // enableOutsideTemperatureCompensation
  OpenThermResponseStatus responseStatus = ot.getLastResponseStatus();
  if (responseStatus != OpenThermResponseStatus::SUCCESS)
  {
    LastboilerResponseError = String(response, HEX);
    sprintf(log_chars, "!!!!!!!!!!!Error: Invalid boiler response %s", LastboilerResponseError.c_str());
    log_message(log_chars);
  } else
  {
    ot.setDHWSetpoint(dhwTarget);
    ot.setBoilerTemperature(op);

    status_CHActive = ot.isCentralHeatingActive(response);
    status_WaterActive = ot.isHotWaterActive(response);
    bool status_flame_tmp = status_FlameOn;
    status_FlameOn = ot.isFlameOn(response);
    if (status_flame_tmp != status_FlameOn) {
      if (status_FlameOn) {
        start_flame_time = millis();
      } else start_flame_time = 0; //After change flame status If flame is on get timer, else reset timer
      flame_time = 0;
    }
    status_Cooling = ot.isCoolingActive(response);
    status_Diagnostic = ot.isDiagnostic(response);
    flame_level = ot.getModulation();
    tempBoiler = ot.getBoilerTemperature();
    tempCWU = ot.getDHWTemperature();
    retTemp = ot.getReturnTemperature();
    pressure = ot.getPressure();
  }

  status_Fault = ot.isFault(response);

}

void IRAM_ATTR handleInterrupt()
{
  ot.handleInterrupt();
}

void getTemp()
{
  if (check_isValidTemp(floor2_tempset) && check_isValidTemp(floor1_tempset)) roomtempSet = (floor2_tempset + floor1_tempset) / 2; //{roomtemp_last=roomtemp; roomtemp=(floor2_tempset+floor1_tempset)/2;}
  if (check_isValidTemp(floor2_tempset) && !check_isValidTemp(floor1_tempset)) {
    roomtempSet = floor2_tempset;
  }
  if (!check_isValidTemp(floor2_tempset) && check_isValidTemp(floor1_tempset)) {
    roomtempSet = floor1_tempset;
  }

  if (check_isValidTemp(floor2_temp) && check_isValidTemp(floor1_temp)) roomtemp = (floor2_temp + floor1_temp) / 2; //{roomtemp_last=roomtemp; roomtemp=(floor2_tempset+floor1_tempset)/2;}
  if (check_isValidTemp(floor2_temp) && !check_isValidTemp(floor1_temp)) {
    roomtemp = floor2_temp;
  }
  if (!check_isValidTemp(floor2_temp) && check_isValidTemp(floor1_temp)) {
    roomtemp = floor1_temp;
  }
  sprintf(log_chars,"Get Temps: floor2_temp: %s/%s, floor1_temp: %s/%s, roomtemp: %s/%s", String(floor2_temp).c_str(), String(floor2_tempset).c_str(), String(floor1_temp).c_str(), String(floor1_tempset).c_str(), String(roomtemp).c_str(),String(roomtempSet).c_str());
  log_message(log_chars);

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
    unsigned long long nowtime = millis();
    float boiler_power = 0;
    if (retTemp < boiler_50_30_ret) boiler_power = boiler_50_30; else boiler_power = boiler_80_60;
    double boilerpower = boiler_power * (flame_level / 100); //kW
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
