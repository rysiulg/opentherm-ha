//received from webserial

void recvMsg(uint8_t *data, size_t len)
{ // for WebSerial
#ifndef enableWebSerialorSerial
  String d = "";
  for (size_t i = 0; i < len; i++)
  {
    d += char(data[i]);
  }
  d.trim();
  d.toUpperCase();
  sprintf(log_chars, "DirectCommands recvMsg Received: %s (dł: %s)", String(d).c_str(), String(d.length()).c_str());
  log_message(log_chars);

  if (d == "RESTART")
  {
    log_message((char*)F("OK. Restarting... by command..."));
    restart();
  }
  if (d == "RECONNECT")
  {
    mqttReconnect();
  }
  if (d == "ROOMTEMP+")
  {
    float startroomtemp = roomtemp;
    if (roomtemp < roomtemphi) {
      roomtemp = roomtemp + 0.5;
    }
    tmanual = true;
    lastTempSet = millis();
    sprintf(log_chars, "Change ROOMTEMP+ from: %s to: %s", String(startroomtemp).c_str(), String(roomtemp).c_str());
    log_message(log_chars);
  }
  if (d == "ROOMTEMP-")
  {
    float startroomtemp = roomtemp;
    if (roomtemp > roomtemplo) {
      roomtemp = roomtemp - 0.5;
    }
    lastTempSet = millis();
    tmanual = true;
    sprintf(log_chars, "Change ROOMTEMP- from: %s to: %s", String(startroomtemp).c_str(), String(roomtemp).c_str());
    log_message(log_chars);
  }
  if (d == "ROOMTEMP0")
  {
    tmanual = !tmanual;
    lastTempSet = millis();
    sprintf(log_chars, "Toggle ROOMTEMP0 from: %s to: %s", String(!tmanual ? "MANUAL" : "AUTO").c_str(), String(tmanual ? "MANUAL" : "AUTO").c_str());
    log_message(log_chars);
  }
  if (d == "SAVE")
  {
    sprintf(log_chars, "Saving config to EEPROM memory by command...  CONFIG Size: %s", String(sizeof(CONFIGURATION)).c_str());
    log_message(log_chars, 0);
    SaveConfig();
  }
  if (d == "RESET_CONFIG")
  {
    sprintf(log_chars, "RESET config to DEFAULT VALUES and restart...  CONFIG Size: %s", String(sizeof(CONFIGURATION)).c_str());
    log_message(log_chars, 0);
    SPIFFS.format();
    CONFIGURATION.version[0] = 'R';
    CONFIGURATION.version[1] = 'E';
    CONFIGURATION.version[2] = 'S';
    CONFIGURATION.version[3] = 'E';
    CONFIGURATION.version[4] = 'T';
    SaveConfig();
    restart();
  }
  if (d == "RESET_FLAMETOTAL" or d == "RFT")
  {
    log_message((char*)F("RESET flame Total var to 0..."), 0);

    flame_used_power_kwh = 0;
    flame_time_total = 0;
    flame_used_power_waterTotal = 0;
    flame_time_waterTotal = 0;
    flame_used_power_CHTotal = 0;
    flame_time_CHTotal = 0;
    SaveConfig();
  }

  if (d == "HELP")
  {
    log_message((char*)F("KOMENDY:\n \
      ROOMTEMP0        -Przelacza temperature z pokoju na automat,\n \
      ROOMTEMP+        -Zwiększa wartość temperatury z pokoju o 0,5 stopnia,\n \
      ROOMTEMP-        -Zmniejsza wartość temperatury z pokoju o 0,5 stopnia,\n \
      RESTART          -Uruchamia ponownie układ,\n \
      RECONNECT        -Dokonuje ponownej próby połączenia z bazami,\n \
      SAVE             -Wymusza zapis konfiguracji,\n \
      RESET_CONFIG     -UWAGA!!!! Resetuje konfigurację do wartości domyślnych\n \
      RESET_FLAMETOTAL -UWAGA!!!! Resetuje licznik płomienia-zużycia kWh na 0"), 0);
  }
#endif
}
String get_PlaceholderName(u_int i)
{
  switch(i) {
    case ASS_uptimedana: return "uptimedana"; break;
    case ASS_temp_NEWS: return "temp_NEWS"; break;
    case ASS_tempBoiler: return "tempBoiler"; break;
    case ASS_tempBoilerSet: return "sliderValue1"; break;
    case ASS_retTemp: return "retTemp"; break;
    case ASS_tempCWU: return "tempCWU"; break;
    case ASS_dhwTarget: return "sliderValue2"; break;
    case ASS_cutOffTemp: return "sliderValue3"; break;
    case ASS_roomtemp: return "roomtemp"; break;
    case ASS_roomtempSet: return "sliderValue4"; break;  //Room Target sp
    case ASS_lastNEWSSet: return "lastNEWSSet"; break;
    case ASS_AutoMode: return "boilermodewww"; break;
    case ASS_EnableHeatingCO: return "boilerwww"; break;
    case ASS_statusWaterActive: return "statusWaterActive"; break;  //pump for water cwu active
    case ASS_statusCHActive: return "statusCHActive"; break;
    case ASS_statusFlameOn: return "statusFlameOn"; break;
    case ASS_statusFault: return "statusFault"; break;
    case ASS_EnableHotWater: return "boilerhwwww"; break;
    case ASS_Statusy: return "Statusy"; break;
    case ASS_UsedMedia: return "UsedMedia"; break;
    case ASS_ecoMode: return "ecoMode"; break;
    case ASS_MemStats: return "MemStats"; break;
  }
  return "\0";
}

void updateDatatoWWW_received(u_int i)
{
  sprintf(log_chars, "Received data nr: %s", String(i).c_str());
  log_message(log_chars);
  switch (i) {
    case ASS_tempBoilerSet:
      tempBoilerSet = PayloadtoValidFloat(ASS[ASS_tempBoilerSet].Value, true, oplo, ophi);
      break;
    case ASS_dhwTarget:
      dhwTarget = PayloadtoValidFloat(ASS[ASS_dhwTarget].Value, true, oplo, ophi);
      break;
    case ASS_cutOffTemp:
      cutOffTemp = PayloadtoValidFloat(ASS[ASS_cutOffTemp].Value, true, cutofflo, cutoffhi);
      lastcutOffTempSet = millis();
      break;
    case ASS_roomtempSet:
        roomtempSet = PayloadtoValidFloat(ASS[ASS_roomtempSet].Value, true, roomtemplo, roomtemphi);
      break;
    case ASS_AutoMode:
      if (PayloadStatus(ASS[ASS_AutoMode].Value, false)) {
        automodeCO = false;
        receivedmqttdata = true;
        //sprintf(log_chars, "CO mode: %s", ASS[ASS_AutoMode].Value.c_str());
        //log_message(log_chars);
      } else
      if (PayloadStatus(ASS[ASS_AutoMode].Value, true)) {
        if (heatingEnabled) {
          automodeCO = true;
          receivedmqttdata = true;
        }
          //sprintf(log_chars, "CO mode: %s", ASS[ASS_AutoMode].Value.c_str());
          //log_message(log_chars);
      } else {
          //sprintf(log_chars, "Unknown mode: %s", ASS[ASS_AutoMode].Value.c_str());
          //log_message(log_chars);
      }
      break;
    case ASS_EnableHeatingCO:
      if (PayloadStatus(ASS[ASS_EnableHeatingCO].Value, true)) {
        heatingEnabled = true;
      } else if (PayloadStatus(ASS[ASS_EnableHeatingCO].Value, false)) {
        heatingEnabled = false;
      } else
      {
        //sprintf(log_chars, "Unknown mode: %s", ASS[ASS_EnableHeatingCO].Value.c_str());
        //log_message(log_chars);
      }
      break;
    case ASS_EnableHotWater:
      if (PayloadStatus(ASS[ASS_EnableHotWater].Value, true)) {
        enableHotWater = true;
      } else if (PayloadStatus(ASS[ASS_EnableHotWater].Value, false)) {
        enableHotWater = false;
      } else
      {
        //sprintf(log_chars, "Unknown mode: %s", ASS[ASS_EnableHotWater].Value.c_str());
        //log_message(log_chars);
      }
    break;
  }
}

void updateDatatoWWW() //default false so if true than update
{
  wdt_reset();
  sprintf(log_chars, "Update Data to www ");
  log_message(log_chars);
#ifdef enableWebSocket
  //if (receivedwebsocketdata and dont_send_after_sync) return;
  //String dana = {"DHWTemp",DHW_Temp}
  String ptr = "\0";

  //tempBoilerSet


    ASS[ASS_uptimedana].Value = String(uptimedana(0));
    ASS[ASS_temp_NEWS].Value = String(temp_NEWS, decimalPlaces);
    ASS[ASS_tempBoiler].Value = String(tempBoiler, decimalPlaces);
    ASS[ASS_tempBoilerSet].Value = String(tempBoilerSet, decimalPlaces);
    ASS[ASS_retTemp].Value = String(retTemp, decimalPlaces);
    ASS[ASS_tempCWU].Value = String(tempCWU, decimalPlaces);
    ASS[ASS_dhwTarget].Value = String(dhwTarget, decimalPlaces);
    ASS[ASS_cutOffTemp].Value = String(cutOffTemp, decimalPlaces);
    ASS[ASS_roomtemp].Value = String(roomtemp, decimalPlaces);
    ASS[ASS_roomtempSet].Value = String(roomtempSet, decimalPlaces);
    ASS[ASS_lastNEWSSet].Value = String(uptimedana(lastNEWSSet));
    ASS[ASS_AutoMode].Value = String(automodeCO ? "on" : "off");
    ASS[ASS_EnableHotWater].Value = String(enableHotWater ? "on" : "off");
    ASS[ASS_statusCHActive].Value = String(status_CHActive ? "on" : "off");
    ASS[ASS_statusWaterActive].Value = String(status_WaterActive ? "on" : "off");
    ASS[ASS_statusFlameOn].Value = String(status_FlameOn ? "on" : "off");
    ASS[ASS_statusFault].Value = String(status_Fault ? "on" : "off");
    ASS[ASS_EnableHeatingCO].Value = String(heatingEnabled ? "on" : "off");
    ptr = "\0";
    if (status_FlameOn) {
      ptr += "<i class='fas fa-fire' style='color: red'></i>"; ptr += "<span class='dht-labels'>" + String(Flame_Active_Flame_level) + "</span><B>" + String(flame_level, 0) + "<sup class=\"units\">&#37;</sup></B>";
      ptr += "<br>";
    }
    if (status_Fault) ptr += "<span class='dht-labels'><B>!!!!!!!!!!!!!!!!! status_Fault !!!!!!!<br></B></span>";
    if (heatingEnabled) {
      ptr += "<span class='dht-labels'><B>" + String(BOILER_HEAT_ON);
      if (automodeCO) ptr += F(" (AUTO)"); else ptr += F(" (Standard)");
      ptr += ("<br></B></span>");
    }
    if (status_CHActive) ptr += "<font color=\"red\"><span class='dht-labels'><B>" + String(BOILER_IS_HEATING) + "<br></B></span></font>";
    if (enableHotWater) ptr += "<span class='dht-labels'><B>" + String(DHW_HEAT_ON) + "<br></B></span>";
    if (status_WaterActive) ptr += "<font color=\"red\"><span class='dht-labels'><B>" + String(Boiler_Active_heat_DHW) + "<br></B></span></font>";
    if (status_Cooling) ptr += "<font color=\"orange\"><span class='dht-labels'><B>" + String(CoolingMode) + "<br></B></span></font>";
    if (status_Diagnostic) ptr += "<font color=\"darkred\"><span class='dht-labels'><B>" + String(DiagMode) + "<br></B></span></font>";
    if (CO_PumpWorking) ptr += "<font color=\"blue\"><span class='dht-labels'><B>" + String(Second_Engine_Heating_PompActive_Disable_heat) + "<br></B><br></span></font>";
    if (Water_PumpWorking) ptr += "<font color=\"blue\"><span class='dht-labels'><B>" + String(Second_Engine_Heating_Water_PompActive) + "<br></B><br></span></font>";
    if (status_FlameOn) ptr += "<font color=\"green\"><span class='dht-labels'>" + String(Flame_time) + "<B>" + uptimedana(flame_time) + "<br></B><br></span></font>";
    ASS[ASS_Statusy].Value = String(ptr);

    ptr = "\0";
    ptr += String(Flame_total) + "<B>" + String(flame_used_power_kwh, 4) + "kWh</B>";
    ptr += String(" : ") + "<B>" + String(uptimedana((flame_time_total), true)+"</B>");
    ptr += "<br>w tym woda: <B>" + String(flame_used_power_waterTotal, 4) + "kWh</B>";
    ptr += String(" : ") + "<B>" + String(uptimedana((flame_time_waterTotal), true)+"</B>");
    ptr += "<br>w tym CO: <B>" + String(flame_used_power_CHTotal, 4) + "kWh</B>";
    ptr += String(" : ") + "<B>" + String(uptimedana((flame_time_CHTotal), true)+"</B>");
    sprintf(log_chars,"Flame_Total: %s (%s), CO: %s (%s), DHW: %s (%s)", String(flame_used_power_kwh).c_str(), String(uptimedana((flame_time_total), true)).c_str(), String(flame_used_power_CHTotal).c_str(), String(uptimedana((flame_time_CHTotal), true)).c_str(), String(flame_used_power_waterTotal).c_str(), String(uptimedana((flame_time_waterTotal), true)).c_str());
    log_message(log_chars);
    ASS[ASS_UsedMedia].Value = String(ptr);


    ptr = "\0";
    ptr += F("Free mem: <B>");
    ptr += String(getFreeMemory());
    ptr += F("&percnt;</B>, Heap: <B>");
    ptr += formatBytes(ESP.getFreeHeap());
    ptr += F("</B><br>Wifi: <B>");
    ptr += String(getWifiQuality());
    ptr += F("&percnt;");
    #ifdef enableMQTT
    ptr += F("</B>, Mqtt reconnects: <B>");
    ptr += mqttReconnects;
    #endif
    ASS[ASS_MemStats].Value = String(ptr);
    ASS[ASS_ecoMode].Value = String(ecoMode ? "on" : "off");

#endif
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
      if (roomtemp < (roomtempSet + 0.6)) COHeat = true; else COHeat = false;
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
