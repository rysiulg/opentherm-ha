/*************************************************************
  This example runs directly on ESP8266 chip.

  Please be sure to select the right ESP8266 module
  in the Tools -> Board -> WeMos D1 Mini

  Adjust settings in Config.h before run
 *************************************************************/

//#include <Arduino.h>
//const float InitTemp = 255;
#include "main.h"


//***********************************************************************************************************************************************************************************************
void IRAM_ATTR Handle_Interrupt()
{
  OpenTherm.handleInterrupt();
}

void IRAM_ATTR processResponse(unsigned long response, OpenThermResponseStatus responseStatus)
{
  OpenTherm.process();
  #ifdef debug
  sprintf(log_chars, "OpenTherm response: %s, OpenTherm OTStatus: %s", String(response).c_str(), OpenTherm.statusToString(responseStatus));
  log_message(log_chars);
#endif
}
//***********************************************************************************************************************************************************************************************
// op = pid(sp, t, t_last, ierr, dt);
float pid(float sp, float pv, float pv_last, float &ierr, float dt)
{ //Generate curve heating based on roomtemp/roomtempset
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
//***********************************************************************************************************************************************************************************************
// This function calculates temperature every second.
void opentherm_update_data()
{
  log_message((char*)F("Update_OpenTherm_data"));
  // Set/Get Boiler Status
  bool enableCooling = false;
  receivedmqttdata = false;
  float histCOtmp = histCO,
        histCWUtmp = histCWU;
  bool COHeat = false;
  if (ecoMode) {opcohi = ecohi;} else {opcohi = opcohistatic;}
  if (tempBoilerSet > opcohi) {tempBoilerSet = opcohi;}

  getTemp(); //default returns roomtemp (avg) and as global sp=roomtempset (avg), roomtemp is also global var
  float op = tempBoilerSet;
  if (heatingEnabled) {
    //  if (!automodeCO) {
    //this will remove tempset avg from floors tempBoilerSet = op_override;  //for no automode
    if (temp_NEWS < cutOffTemp)
      COHeat = true;
    if (CO_PumpWorking)
      COHeat = false;
    if (temp_NEWS > (cutOffTemp + cutoff_histereza))
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

  #ifdef debug
  sprintf(log_chars,"Statusy openth: COHEAT: %s, DHW: %s, automodeCO: %s, temp_NEWS: %f", COHeat?"chodzi":"stoi", String(enableHotWater?"chodzi":"stoi").c_str(), String(automodeCO?"ON":"stoi").c_str(), temp_NEWS);
  log_message(log_chars);
  #endif

  #ifdef debug
  log_message((char*)F("Set statuses from dhwTarget"));
  #endif
  if (status_WaterActive) {histCWUtmp = 0;} //reset to heat longer (to Set temp) if active -on next request
  OpenTherm.setDHWSetpoint(dhwTarget - histCWUtmp);
  #ifdef debug
  log_message((char*)F("Set statuses from op"));
  #endif
  if (status_CHActive) {histCOtmp = 0;} //reset to increase heat to Set temperature if active CO -on next request
  OpenTherm.setBoilerTemperature(op - histCOtmp);

  #ifdef debug
  log_message((char*)F("Set statuses from isFlameOn"));
  #endif
  if (status_flame_prv != status_FlameOn) {
    if (status_FlameOn) {
      start_flame_time = millis();
      start_flame_time_fordisplay = start_flame_time;
    } else {
      start_flame_time = 0;
      start_flame_time_fordisplay = start_flame_time;//After change flame status If flame is on get timer, else reset timer
    }
  }

  unsigned long  response = 0;
  if (OpenTherm.isReady()) {
    log_message((char*)F("OpenTherm make request"));
    response = OpenTherm.setBoilerStatus(COHeat, enableHotWater, enableCooling); // enableOutsideTemperatureCompensation

    OpenThermResponseStatus responseStatus = OpenTherm.getLastResponseStatus();
    LastboilerResponse = String(response, HEX);
    LastboilerResponse += F(", Status: ");
    LastboilerResponse += OpenTherm.statusToString(responseStatus);

    if (responseStatus != OpenThermResponseStatus::SUCCESS)
    {
      sprintf(log_chars, "!!!!!!!!!!!Error: Invalid boiler response Error: %s", LastboilerResponse.c_str());
      log_message(log_chars);
    } else
    {
      Serial.println("Response is: "+ OpenTherm.isValidResponse(response)?"Valid":"No VALID");
      #ifdef debug
      log_message((char*)F("Set statuses from opentherm"));
      #endif
      status_CHActive = OpenTherm.isCentralHeatingActive(response);
      #ifdef debug
      sprintf(log_chars,"Set statuses from isCentralHeatingActive: %s", status_CHActive?"Yes":"No");
      log_message(log_chars);
      #endif
      status_WaterActive = OpenTherm.isHotWaterActive(response);
      #ifdef debug
      sprintf(log_chars,"Set statuses from isHotWaterActive: %s", status_WaterActive?"Yes":"No");
      log_message(log_chars);
      #endif
      status_flame_prv = status_FlameOn;
      status_FlameOn = OpenTherm.isFlameOn(response);
      #ifdef debug
      sprintf(log_chars,"Set statuses from status_FlameOn: %s", status_FlameOn?"Yes":"No");
      log_message(log_chars);
      #endif
      status_Cooling = OpenTherm.isCoolingActive(response);
      #ifdef debug
      sprintf(log_chars,"Set statuses from isCoolingActive: %s", status_Cooling?"Yes":"No");
      log_message(log_chars);
      #endif
      status_Diagnostic = OpenTherm.isDiagnostic(response);
      #ifdef debug
      sprintf(log_chars,"Set statuses from isDiagnostic: %s", status_Diagnostic?"Yes":"No");
      log_message(log_chars);
      #endif
      flame_level = OpenTherm.getModulation();
      #ifdef debug
      sprintf(log_chars,"Set statuses from getModulation: %f", flame_level);
      log_message(log_chars);
      #endif
      tempBoiler = OpenTherm.getBoilerTemperature();
      #ifdef debug
      sprintf(log_chars, "Set statuses from getBoilerTemperature: %f", tempBoiler);
      log_message(log_chars);
      #endif
      tempCWU = OpenTherm.getDHWTemperature();
      #ifdef debug
      sprintf(log_chars,"Set statuses from getDHWTemperature: %f", tempCWU);
      log_message(log_chars);
      #endif
      retTemp = OpenTherm.getReturnTemperature();
      #ifdef debug
      sprintf(log_chars,"Set statuses from getReturnTemperature: %f", retTemp);
      log_message(log_chars);
      #endif
      pressure = OpenTherm.getPressure();
      #ifdef debug
      sprintf(log_chars,"Set statuses from getPressure: %f",pressure);
      log_message(log_chars);

      sprintf(log_chars,"getVentilation: %d\ngetVentSupplyInTemperature: %f\ngetVentSupplyOutTemperature: %f\n, getVentExhaustInTemperature: %f\ngetVentExhaustOutTemperature: %f",OpenTherm.getVentilation(), OpenTherm.getVentSupplyInTemperature(), OpenTherm.getVentSupplyOutTemperature(), OpenTherm.getVentExhaustInTemperature(),OpenTherm.getVentExhaustOutTemperature());
      log_message(log_chars);
      sprintf(log_chars,"getFaultIndication: %i\ngetVentilationMode: %i\ngetBypassStatus: %d\ngetBypassAutomaticStatus: %i\ngetDiagnosticIndication: %i\n", OpenTherm.getFaultIndication()?1:0, OpenTherm.getVentilationMode()?1:0, OpenTherm.getBypassStatus()?1:0, OpenTherm.getBypassAutomaticStatus()?1:0, OpenTherm.getDiagnosticIndication()?1:0);
      log_message(log_chars);
      sprintf(log_chars,"getOutsideTemperature: %f\ngetDHWFlowrate: %f", OpenTherm.getOutsideTemperature(), OpenTherm.getDHWFlowrate());
      log_message(log_chars);
      #endif
    }
    status_Fault = OpenTherm.isFault(response);
    #ifdef debug
    if (status_Fault) log_message((char*)F("OpenTherm Set fault status from opentherm"));
    #endif
  }
}

void getTemp() {
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
  sprintf(log_chars,"getTemp: floor2_temp: %f/%f, floor1_temp: %f/%f, roomtemp: %f/%f", floor2_temp, floor2_tempset, floor1_temp, floor1_tempset, roomtemp, roomtempSet);
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
  OpenTherm.begin(Handle_Interrupt, processResponse);

  // Init DS18B20 sensor
  sensors.begin();
  sensors.requestTemperatures();
  sensors.setWaitForConversion(false); // switch to async mode
  dallasTemp = sensors.getTempCByIndex(0);
  if (check_isValidTemp(dallasTemp)) dallasTemp = InitTemp;
  sprintf(log_chars,"Dallas Temp: %s", String(dallasTemp).c_str());
  log_message(log_chars);
  //  lastTempSet = -extTempTimeout_ms;
#ifdef debug
  log_message((char*)F("end setup...."));
  //    SaveConfig();
#endif
}

void loop()
{
  // Serial.println("loop start.");
  MainCommonLoop();
  // Serial.println("loop local.");
  unsigned long now = millis(); // TO AVOID compare -2>10000 which is true ??? why?
  // check mqtt is available and connected in other case check values in api.

   if ((now - lastUpdate) > statusUpdateInterval_ms)
   {
     lastUpdate = now;
    opentherm_update_data(); // According OpenTherm Specification from Ihnor Melryk Master requires max 1s interval communication -przy okazji wg czasu update mqtt zrobie odczyt dallas
   }

  if (status_FlameOn) {  //calculate statistics
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

  if (((now - lastSaveConfig) > save_config_every) || lastSaveConfig == 0)
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
  if (!check_isValidTemp(temp_NEWS) && check_isValidTemp(dallasTemp) && lastNEWSSet == 0) temp_NEWS = dallasTemp;

  }
// Serial.println("loop end1.");
// wdt_reset();
// Serial.println("loop end2.");
// loop();
}
