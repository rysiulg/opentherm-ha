//received from webserial

void RemoteCommandReceived(uint8_t *data, size_t len)
{ // for WebSerial
#ifndef enableWebSerialorSerial
  String d = "\0";
  for (size_t i = 0; i < len; i++)
  {
    d += char(data[i]);
  }
  d.trim();
  d.toUpperCase();
  log_message((char*)F("------------------------------------------------------------------------------------------------"));
  #ifdef debug
  sprintf(log_chars, "DirectCommands RemoteCommandReceived Received: %s (dł: %s)", String(d).c_str(), String(d.length()).c_str());
  log_message(log_chars);
  #endif

  if (d == "RESTART")
  {
    log_message((char*)F("OK. Restarting... by command..."));
    restart(F("Initiated from Remote Command RESTART..."));
  } else
  if (d == "SENDLOGTOMQTT")
  {
    sendlogtomqtt = !sendlogtomqtt;
    // Serial.print("sendlogtomqtt: ");
    // Serial.println(sendlogtomqtt);
    sprintf(log_chars,"Toggle Value sending log to MQTT. Actual Value: %s, %s", String((sendlogtomqtt)? "MQTT LOG" : "DISABLED").c_str(), String(sendlogtomqtt).c_str());
    log_message(log_chars);
  } else
  if (d == "RECONNECT")
  {
    log_message((char*)F("OK. Try reconnect mqtt"));
    #ifdef enableMQTT
    mqttReconnect();
    #endif
  } else
  if (d == "LOAD")
  {
    if(SPIFFS.exists(configfile)) {
      String dane = "\0";
      File file = SPIFFS.open(configfile,"r");
      while (file.available()) {
        dane += (file.readStringUntil('\n'));
      }
      file.close();
      loadConfig();
    }
  } else
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
  } else
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
  } else
  if (d == "ROOMTEMP0")
  {
    tmanual = !tmanual;
    lastTempSet = millis();
    sprintf(log_chars, "Toggle ROOMTEMP0 from: %s to: %s", String(!tmanual ? "MANUAL" : "AUTO").c_str(), String(tmanual ? "MANUAL" : "AUTO").c_str());
    log_message(log_chars);
  } else
  if (d == "SAVE")
  {
    sprintf(log_chars, "Saving config to EEPROM memory by command... ");
    log_message(log_chars, 0);
    SaveConfig();
  } else
  if (d == "RESET_CONFIG")
  {
    sprintf(log_chars, "RESET ALL config to DEFAULT VALUES with all www content and restart...  ");
    log_message(log_chars, 0);
    SPIFFS.format();
    for (u_int i=0 ; i < 100; i++){
      EEPROM.put(i, "\0");
    }

    SaveConfig();
    restart(F("Initiated from Remote Command RESET_CONFIG..."));
  } else
  if (d == "RESET_FLAMETOTAL" or d == "RFT")
  {
    log_message((char*)F("RESET flame Total vars and CRT to 0..."));

    flame_used_power_kwh = 0;
    flame_time_total = 0;
    flame_used_power_waterTotal = 0;
    flame_time_waterTotal = 0;
    flame_used_power_CHTotal = 0;
    flame_time_CHTotal = 0;
    CRTrunNumber = 0;
    SaveConfig();
  } else
  if (d.indexOf("HELP")>=0)
  {
    String d1 = d;
    d1.replace("HELP","");
    d1.trim();
    if (d1.length()==0) {
      log_message((char*)F("HELP MENU.--------------------------------------------------------------------------------------"));
      log_message((char*)F("KOMENDY: RECONNECT, SAVE, RESET_CONFIG, RESTART, RESET_FLAMETOTAL, SENDLOGTOMQTT, ROOMTEMP0, ROOMTEMP+, ROOMTEMP-"));
      log_message((char*)F("Dodatkowa pomoc dot. komendy po wpisaniu jej wartości np. HELP SAVE"));
    } else
    {
      if (d.indexOf(F("RECONNECT")) >=0) {
        log_message((char*)F(" RECONNECT   -Dokonuje ponownej próby połączenia z bazami,"));
      } else
      if (d.indexOf(F("SENDLOGTOMQTT")) >=0) {
        log_message((char*)F(" SENDLOGTOMQTT   -Wysyła Logi do serwera MQTT w topiku log."));
      } else
      if (d.indexOf("LOAD") >=0) {
        log_message((char*)F(" LOAD    -Wymusza odczyt konfiguracji,"));
      } else
      if (d.indexOf("SAVE") >=0) {
        log_message((char*)F(" SAVE    -Wymusza zapis konfiguracji,"));
      } else
      if (d.indexOf("RESET_CONFIG") >=0) {
        log_message((char*)F(" RESET_CONFIG    -UWAGA!!!! Resetuje konfigurację do wartości domyślnych wraz z plikami www (konieczne ponowne wgranie index.html),"));
      } else
      if (d.indexOf("RESET_FLAMETOTAL") >=0) {
        log_message((char*)F("  RESET_FLAMETOTAL  -UWAGA!!!! Resetuje licznik płomienia-zużycia kWh i czas oraz CRT na 0"));
      } else
      if (d.indexOf("ROOMTEMP0") >=0) {
        log_message((char*)F("  ROOMTEMP0   -Przelacza temperature z pokoju na automat,"));
      } else
      if (d.indexOf("ROOMTEMP+") >=0) {
        log_message((char*)F(" ROOMTEMP+  -Zwiększa wartość temperatury z pokoju o 0,5 stopnia,"));
      } else
      if (d.indexOf("ROOMTEMP-") >=0) {
        log_message((char*)F(" ROOMTEMP-  -Zmniejsza wartość temperatury z pokoju o 0,5 stopnia,"));
      } else
      if (d.indexOf("RESTART") >=0) {
        log_message((char*)F(" RESTART  -Uruchamia ponownie układ,"));
      }
    }
  } else
  log_message((char*)F("Unknown command received from serial or webserial input."));
  log_message((char*)F("------------------------------------------------------------------------------------------------"));

#endif
}
String get_PlaceholderName(u_int i)
{ //get names by number to match www placeholders
  switch(i) {
    case ASS_uptimedana: return PSTR("uptimedana"); break;
    case ASS_temp_NEWS: return PSTR("temp_NEWS"); break;
    case ASS_tempBoiler: return PSTR("tempBoiler"); break;
    case ASS_tempBoilerSet: return PSTR("sliderValue1"); break;
    case ASS_retTemp: return PSTR("retTemp"); break;
    case ASS_tempCWU: return PSTR("tempCWU"); break;
    case ASS_dhwTarget: return PSTR("sliderValue2"); break;
    case ASS_cutOffTemp: return PSTR("sliderValue3"); break;
    case ASS_roomtemp: return PSTR("roomtemp"); break;
    case ASS_roomtempSet: return PSTR("sliderValue4"); break;  //Room Target sp
    case ASS_lastNEWSSet: return PSTR("lastNEWSSet"); break;
    case ASS_AutoMode: return PSTR("boilermodewww"); break;
    case ASS_EnableHeatingCO: return PSTR("boilerwww"); break;
    case ASS_statusWaterActive: return PSTR("statusWaterActive"); break;  //pump for water cwu active
    case ASS_statusCHActive: return PSTR("statusCHActive"); break;
    case ASS_statusFlameOn: return PSTR("statusFlameOn"); break;
    case ASS_statusFault: return PSTR("statusFault"); break;
    case ASS_EnableHotWater: return PSTR("boilerhwwww"); break;
    case ASS_Statusy: return PSTR("Statusy"); break;
    case ASS_UsedMedia: return PSTR("UsedMedia"); break;
    case ASS_ecoMode: return PSTR("ecoMode"); break;
    case ASS_MemStats: return PSTR("MemStats"); break;
    case ASS_opcohi: return PSTR("opcohi"); break;
  }
  return "\0";
}

void updateDatatoWWW_received(u_int i) {
  wdt_reset();
  sprintf(log_chars, "Received data nr: %s", String(i).c_str());
  log_message(log_chars);
  switch (i) {
    case ASS_tempBoilerSet:
      tempBoilerSet = PayloadtoValidFloat(String(ASS[ASS_tempBoilerSet].Value), true, oplo, ophi);
      break;
    case ASS_dhwTarget:
      dhwTarget = PayloadtoValidFloat(String(ASS[ASS_dhwTarget].Value), true, oplo, ophi);
      break;
    case ASS_cutOffTemp:
      cutOffTemp = PayloadtoValidFloat(String(ASS[ASS_cutOffTemp].Value), true, cutofflo, cutoffhi);
      lastcutOffTempSet = millis();
      break;
    case ASS_roomtempSet:
        roomtempSet = PayloadtoValidFloat(String(ASS[ASS_roomtempSet].Value), true, roomtemplo, roomtemphi);
      break;
    case ASS_AutoMode:
      if (PayloadStatus(String(ASS[ASS_AutoMode].Value), false)) {
        automodeCO = false;
        receivedmqttdata = true;
        //sprintf(log_chars, "CO mode: %s", ASS[ASS_AutoMode].Value.c_str());
        //log_message(log_chars);
      } else
      if (PayloadStatus(String(ASS[ASS_AutoMode].Value), true)) {
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
      if (PayloadStatus(String(ASS[ASS_EnableHeatingCO].Value), true)) {
        heatingEnabled = true;
      } else if (PayloadStatus(String(ASS[ASS_EnableHeatingCO].Value), false)) {
        heatingEnabled = false;
      } else
      {
        //sprintf(log_chars, "Unknown mode: %s", ASS[ASS_EnableHeatingCO].Value.c_str());
        //log_message(log_chars);
      }
      break;
    case ASS_ecoMode:
      if (PayloadStatus(String(ASS[ASS_ecoMode].Value), true)) {
        ecoMode = true;
        opcohi = ecohi;
      } else if (PayloadStatus(String(ASS[ASS_ecoMode].Value), false)) {
        ecoMode = false;
        opcohi = opcohistatic;
      } else
      {
        //sprintf(log_chars, "Unknown mode: %s", ASS[ASS_EnableHeatingCO].Value.c_str());
        //log_message(log_chars);
      }
      if (tempBoilerSet > opcohi) tempBoilerSet = opcohi;
      break;
      case ASS_EnableHotWater:
      if (PayloadStatus(String(ASS[ASS_EnableHotWater].Value), true)) {
        enableHotWater = true;
      } else if (PayloadStatus(String(ASS[ASS_EnableHotWater].Value), false)) {
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
  String ptrS = "\0";

  //tempBoilerSet
    if (ecoMode) opcohi = ecohi; else opcohi = opcohistatic;
    if (tempBoilerSet > opcohi) tempBoilerSet = opcohi;

    SaveAssValue(ASS_lastNEWSSet,         uptimedana(lastNEWSSet) );
    SaveAssValue(ASS_temp_NEWS,           String(temp_NEWS, decimalPlaces) );
    SaveAssValue(ASS_tempBoiler,          String(tempBoiler, decimalPlaces) );
    SaveAssValue(ASS_tempBoilerSet,       String(tempBoilerSet, decimalPlaces) );
    SaveAssValue(ASS_retTemp,             String(retTemp, decimalPlaces) );
    SaveAssValue(ASS_tempCWU,             String(tempCWU, decimalPlaces) );
    SaveAssValue(ASS_dhwTarget,           String(dhwTarget, decimalPlaces) );
    SaveAssValue(ASS_cutOffTemp,          String(cutOffTemp, decimalPlaces) );
    SaveAssValue(ASS_roomtemp,            String(roomtemp, decimalPlaces) );
    SaveAssValue(ASS_roomtempSet,         String(roomtempSet, decimalPlaces) );
    SaveAssValue(ASS_opcohi,              String(opcohi, decimalPlaces) );
    SaveAssValue(ASS_AutoMode,            automodeCO ? "on" : "off" );
    SaveAssValue(ASS_EnableHotWater,      enableHotWater ? "on" : "off" );
    SaveAssValue(ASS_statusCHActive,      status_CHActive ? "on" : "off" );
    SaveAssValue(ASS_statusWaterActive,   status_WaterActive ? "on" : "off" );
    SaveAssValue(ASS_statusFlameOn,       status_FlameOn ? "on" : "off" );
    SaveAssValue(ASS_statusFault,         status_Fault ? "on" : "off" );
    SaveAssValue(ASS_EnableHeatingCO,     heatingEnabled ? "on" : "off" );
    SaveAssValue(ASS_ecoMode,             ecoMode ? "on" : "off" );

    ptrS = "\0";
    if (status_FlameOn) {
      ptrS += "<i class='fas fa-fire' id='StatusRed'></i>"; ptrS += "<span id='StatusRedNormal'>" + String(Flame_Active_Flame_level) + "</span><b>" + String(flame_level, 0) + "<sup class=\"units\">&#37;</sup></b></br>";
    }
    if (status_Fault) ptrS += "<span id='StatusRed'>!!!!!!!!!!!!!!!!! status_Fault !!!!!!!</span></br>";
    if (heatingEnabled) {
      ptrS += "<span is='StatusBlack'>" + String(BOILER_HEAT_ON);
      if (automodeCO) ptrS += F(" (AUTO)"); else ptrS += F(" (Standard)");
      ptrS += ("</span></br>");
    }
    if (status_CHActive) ptrS += "<span id='StatusRed'>" + String(BOILER_IS_HEATING) + "</span></br>";
    if (enableHotWater) ptrS += "<span id='StatusBlack'>" + String(DHW_HEAT_ON) + "</span></br>";
    if (status_WaterActive) ptrS += "<span id='StatusRed'>" + String(Boiler_Active_heat_DHW) + "</span></br>";
    if (status_Cooling) ptrS += "<span id='StatusOrange'>" + String(CoolingMode) + "</span><</br>";
    if (status_Diagnostic) ptrS += "<span id='StatusDarkRed'>" + String(DiagMode) + "</span></br>";
    if (CO_PumpWorking) ptrS += "<span id='StatusBlue'>" + String(Second_Engine_Heating_PompActive_Disable_heat) + "</span>></br>";
    if (Water_PumpWorking) ptrS += "<span id='StatusBlue'>" + String(Second_Engine_Heating_Water_PompActive) + "</span></br>";
    if (status_FlameOn) ptrS += "<span id='StatusGreen'>" + String(Flame_time) + "<b>" + uptimedana(start_flame_time_fordisplay) + "</b></span></br>";
//    ASS[ASS_Statusy].Value = toCharPointer(String(ptrS));
    //strcpy(ASS[ASS_Statusy].Value, toCharPointer(ptrS) );
    SaveAssValue(ASS_Statusy, ptrS );

    ptrS = "\0";
    ptrS += "<p id='StatusBlackNormal'>" + String(Flame_total) + "<b>" + String(flame_used_power_kwh, 4) + "kWh</b>";
    ptrS += String(" : ") + "<b>" + String(uptimedana((flame_time_total), true)+"</b>");
    ptrS += "</br>w tym woda: <b>" + String(flame_used_power_waterTotal, 4) + "kWh</b>";
    ptrS += String(" : ") + "<b>" + String(uptimedana((flame_time_waterTotal), true)+"</b>");
    ptrS += "</br>w tym CO: <b>" + String(flame_used_power_CHTotal, 4) + "kWh</b>";
    ptrS += String(" : ") + "<b>" + String(uptimedana((flame_time_CHTotal), true)+"</b></p>");
// //    ASS[ASS_UsedMedia].Value = toCharPointer(String(ptrS));
//     strcpy(ASS[ASS_UsedMedia].Value, toCharPointer( ptrS) );
    SaveAssValue(ASS_UsedMedia, ptrS );
    #ifdef debug
    sprintf(log_chars,"Flame_Total: %s (%s), CO: %s (%s), DHW: %s (%s)", String(flame_used_power_kwh).c_str(), String(uptimedana((flame_time_total), true)).c_str(), String(flame_used_power_CHTotal).c_str(), String(uptimedana((flame_time_CHTotal), true)).c_str(), String(flame_used_power_waterTotal).c_str(), String(uptimedana((flame_time_waterTotal), true)).c_str());
    log_message(log_chars);
    #endif
#endif
}

String local_specific_web_processor_vars(String var) {
  if (var == "COPUMP_GET_TOPIC") { return String(COPUMP_GET_TOPIC);
  } else
  if (var == "COPumpStatus_json") { return String(COPumpStatus_json);
  } else
  if (var == "WaterPumpStatus_json") { return String(WaterPumpStatus_json);
  } else
  if (var == "ROOMS_F1_GET_TOPIC") { return String(ROOMS_F1_GET_TOPIC);
  } else
  if (var == "roomF1temp_json") { return String(roomF1temp_json);
  } else
  if (var == "roomF1tempset_json") { return String(roomF1tempset_json);
  } else
  if (var == "ROOMS_F2_GET_TOPIC") { return String(ROOMS_F2_GET_TOPIC);
  } else
  if (var == "roomF2temp_json") { return String(roomF2temp_json);
  } else
  if (var == "roomF2tempset_json") { return String(roomF2tempset_json);
  }
  return "\0";
}
