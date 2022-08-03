

void addusage_local_values_load(String dane, int EpromPosition) {
    // is it correct?

  EEPROM.get(EpromPosition, flame_time_total);
  EpromPosition += sizeof(flame_time_total);
  EEPROM.get(EpromPosition, flame_time_waterTotal);
  EpromPosition += sizeof(flame_time_waterTotal);
  EEPROM.get(EpromPosition, flame_time_CHTotal);
  EpromPosition += sizeof(flame_time_CHTotal);
  EEPROM.get(EpromPosition, flame_used_power_kwh);
  EpromPosition += sizeof(flame_used_power_kwh);
  EEPROM.get(EpromPosition, flame_used_power_waterTotal);
  EpromPosition += sizeof(flame_used_power_waterTotal);
  EEPROM.get(EpromPosition, flame_used_power_CHTotal);

  if (isnan(flame_used_power_kwh) || (flame_used_power_kwh + 1) == 0) flame_used_power_kwh = 0;
  if (isnan(flame_time_total) || (flame_time_total + 1) == 0) flame_time_total = 0;
  if (isnan(flame_time_waterTotal) || (flame_time_waterTotal + 1) == 0) flame_time_waterTotal = 0;
  if (isnan(flame_time_CHTotal) || (flame_time_CHTotal + 1) == 0) flame_time_CHTotal = 0;
  if (isnan(flame_used_power_waterTotal) || (flame_used_power_waterTotal + 1) == 0) flame_used_power_waterTotal = 0;
  if (isnan(flame_used_power_CHTotal) || (flame_used_power_CHTotal + 1) == 0) flame_used_power_CHTotal = 0;

  String tmpstrval = "\0";
  if (dane.indexOf("COPUMP_GET_TOPIC") >= 0)      { tmpstrval = getJsonVal(dane, "COPUMP_GET_TOPIC"); tmpstrval.replace("\"",""); COPUMP_GET_TOPIC = tmpstrval; }
  if (dane.indexOf("COPumpStatus_json") >= 0)     { tmpstrval = getJsonVal(dane, "COPumpStatus_json"); tmpstrval.replace("\"",""); COPumpStatus_json = tmpstrval; }
  if (dane.indexOf("WaterPumpStatus_json") >= 0)  { tmpstrval = getJsonVal(dane, "WaterPumpStatus_json"); tmpstrval.replace("\"",""); WaterPumpStatus_json = tmpstrval; }
  if (dane.indexOf("ROOMS_F1_GET_TOPIC") >= 0)    { tmpstrval = getJsonVal(dane, "ROOMS_F1_GET_TOPIC"); tmpstrval.replace("\"",""); ROOMS_F1_GET_TOPIC = tmpstrval; }
  if (dane.indexOf("roomF1temp_json") >= 0)       { tmpstrval = getJsonVal(dane, "roomF1temp_json"); tmpstrval.replace("\"",""); roomF1temp_json = tmpstrval; }
  if (dane.indexOf("roomF1tempset_json") >= 0)    { tmpstrval = getJsonVal(dane, "roomF1tempset_json"); tmpstrval.replace("\"",""); roomF1tempset_json = tmpstrval; }
  if (dane.indexOf("ROOMS_F2_GET_TOPIC") >= 0)    { tmpstrval = getJsonVal(dane, "ROOMS_F2_GET_TOPIC"); tmpstrval.replace("\"",""); ROOMS_F2_GET_TOPIC = tmpstrval; }
  if (dane.indexOf("roomF2temp_json") >= 0)       { tmpstrval = getJsonVal(dane, "roomF2temp_json"); tmpstrval.replace("\"",""); roomF2temp_json = tmpstrval; }
  if (dane.indexOf("roomF2tempset_json") >= 0)    { tmpstrval = getJsonVal(dane, "roomF2tempset_json"); tmpstrval.replace("\"",""); roomF2tempset_json = tmpstrval; }

  unsigned long long tmpval = 0;
  char * pEnd;
    if (dane.indexOf("flame_time_total") >= 0) {
      tmpval = strtoull (getJsonVal(dane, "flame_time_total").c_str(), &pEnd, 10);
      sprintf(log_chars,"flame_time_total: %s, afterstrtoull: %s, double: %s", String(getJsonVal(dane, "flame_time_total")).c_str(), String(tmpval).c_str(), String(flame_time_total).c_str());
      log_message(log_chars);
      if (tmpval > flame_time_total) flame_time_total = tmpval; //if eprom is reset
    }
    if (dane.indexOf("flame_time_waterTotal") >= 0) {
      tmpval = strtoull (getJsonVal(dane, "flame_time_waterTotal").c_str(), &pEnd, 10);
      if (tmpval > flame_time_waterTotal) flame_time_waterTotal = tmpval; //if eprom is reset
    }
    if (dane.indexOf("flame_time_CHTotal") >= 0) {
      tmpval = strtoull(getJsonVal(dane, "flame_time_CHTotal").c_str(), &pEnd, 10);
      if (tmpval > flame_time_CHTotal) flame_time_CHTotal = tmpval; //if eprom is reset
    }
    double tmpdouble = 0;
    if (dane.indexOf("flame_used_power_kwh") >= 0) {
      tmpdouble = getJsonVal(dane, "flame_used_power_kwh").toDouble();
      if (tmpdouble > flame_used_power_kwh) flame_used_power_kwh = tmpdouble; //if eprom is reset
    }
    if (dane.indexOf("flame_used_power_waterTotal") >= 0) {
      tmpdouble = getJsonVal(dane, "flame_used_power_waterTotal").toDouble();
      if (tmpdouble > flame_used_power_waterTotal) flame_used_power_waterTotal = tmpdouble; //if eprom is reset
    }
    if (dane.indexOf("flame_used_power_CHTotal") >= 0) {
      tmpdouble = getJsonVal(dane, "flame_used_power_CHTotal").toDouble();
      if (tmpdouble > flame_used_power_CHTotal) flame_used_power_CHTotal = tmpdouble; //if eprom is reset
    }

      if (dane.indexOf("heatingEnabled") >= 0)        { tmpstrval = getJsonVal(dane, "heatingEnabled"); tmpstrval.replace("\"",""); heatingEnabled = (bool) (tmpstrval == "1"); }
      if (dane.indexOf("enableHotWater") > -1)        { tmpstrval = getJsonVal(dane, "enableHotWater"); tmpstrval.replace("\"",""); enableHotWater = (bool) (tmpstrval == "1"); }
      if (dane.indexOf("automodeCO") > -1)            { tmpstrval = getJsonVal(dane, "automodeCO"); tmpstrval.replace("\"",""); automodeCO = (bool) (tmpstrval == "1"); }
      if (dane.indexOf("ecoMode") > -1)               { tmpstrval = getJsonVal(dane, "ecoMode"); tmpstrval.replace("\"","");  ecoMode = (bool) (tmpstrval == "1"); }

      if (dane.indexOf("tempBoilerSet") > -1)         { tmpstrval = getJsonVal(dane, "tempBoilerSet"); tmpstrval.replace("\"","");  tempBoilerSet = (float) tmpstrval.toFloat(); }
      if (dane.indexOf("cutOffTemp") > -1)            { tmpstrval = getJsonVal(dane, "cutOffTemp"); tmpstrval.replace("\"","");  cutOffTemp = (float) tmpstrval.toFloat(); }
      if (dane.indexOf("dhwTarget") > -1)             { tmpstrval = getJsonVal(dane, "dhwTarget"); tmpstrval.replace("\"","");  dhwTarget = (float) tmpstrval.toFloat(); }
      if (dane.indexOf("histCWU") > -1)             { tmpstrval = getJsonVal(dane, "histCWU"); tmpstrval.replace("\"","");  histCWU = (float) tmpstrval.toFloat(); }
      if (dane.indexOf("histCO") > -1)             { tmpstrval = getJsonVal(dane, "histCO"); tmpstrval.replace("\"","");  histCO = (float) tmpstrval.toFloat(); }

}

String addusage_local_values_save(int EpromPosition = 0) {
  String retval = "\0";
  retval += F(",\"COPUMP_GET_TOPIC\":\"");
  retval += String(COPUMP_GET_TOPIC);
  retval += F("\"");
  retval += F(",\"COPumpStatus_json\":\"");
  retval += String(COPumpStatus_json);
  retval += F("\"");
  retval += F(",\"WaterPumpStatus_json\":\"");
  retval += String(WaterPumpStatus_json);
  retval += F("\"");
  retval += F(",\"ROOMS_F1_GET_TOPIC\":\"");
  retval += String(ROOMS_F1_GET_TOPIC);
  retval += F("\"");
  retval += F(",\"roomF1temp_json\":\"");
  retval += String(roomF1temp_json);
  retval += F("\"");
  retval += F(",\"roomF1tempset_json\":\"");
  retval += String(roomF1tempset_json);
  retval += F("\"");
  retval += F(",\"ROOMS_F2_GET_TOPIC\":\"");
  retval += String(ROOMS_F2_GET_TOPIC);
  retval += F("\"");
  retval += F(",\"roomF2temp_json\":\"");
  retval += String(roomF2temp_json);
  retval += F("\"");
  retval += F(",\"roomF2tempset_json\":\"");
  retval += String(roomF2tempset_json);
  retval += F("\"");
  retval += ",\"flame_time_total\":";
  retval += String(flame_time_total);
  retval += ",\"flame_time_waterTotal\":";
  retval += String(flame_time_waterTotal);
  retval += ",\"flame_time_CHTotal\":";
  retval += String(flame_time_CHTotal);
  retval += ",\"flame_used_power_kwh\":";
  retval += String(flame_used_power_kwh);
  retval += ",\"flame_used_power_waterTotal\":";
  retval += String(flame_used_power_waterTotal);
  retval += ",\"flame_used_power_CHTotal\":";
  retval += String(flame_used_power_CHTotal);
  retval += ",\"heatingEnabled\":";
  retval += String(heatingEnabled);
  retval += ",\"enableHotWater\":";
  retval += String(enableHotWater);
  retval += ",\"automodeCO\":";
  retval += String(automodeCO);
  retval += ",\"ecoMode\":";
  retval += String(ecoMode);
  retval += ",\"tempBoilerSet\":";
  retval += String(tempBoilerSet);
  retval += ",\"cutOffTemp\":";
  retval += String(cutOffTemp);
  retval += ",\"dhwTarget\":";
  retval += String(dhwTarget);
  retval += ",\"histCWU\":";
  retval += String(histCWU);
  retval += ",\"histCO\":";
  retval += String(histCO);

  if (EpromPosition > 0) {
   unsigned long long runtmp_ull = 0;
   double runtmp_d = 0;
   EEPROM.get(EpromPosition,runtmp_ull);
   if (runtmp_ull != flame_time_total) EEPROM.put(EpromPosition, flame_time_total);
   EpromPosition += sizeof(flame_time_total);
   EEPROM.get(EpromPosition,runtmp_ull);
   if (runtmp_ull != flame_time_waterTotal) EEPROM.put(EpromPosition, flame_time_waterTotal);
   EpromPosition += sizeof(flame_time_waterTotal);
   EEPROM.get(EpromPosition,runtmp_ull);
   if (runtmp_ull != flame_time_CHTotal) EEPROM.put(EpromPosition, flame_time_CHTotal);
   EpromPosition += sizeof(flame_time_CHTotal);

   EEPROM.get(EpromPosition,runtmp_d);
   if (runtmp_d != flame_used_power_kwh) EEPROM.put(EpromPosition, flame_used_power_kwh);
   EpromPosition += sizeof(flame_used_power_kwh);
   EEPROM.get(EpromPosition,runtmp_d);
   if (runtmp_d != flame_used_power_waterTotal) EEPROM.put(EpromPosition, flame_used_power_waterTotal);
   EpromPosition += sizeof(flame_used_power_waterTotal);
   EEPROM.get(EpromPosition,runtmp_d);
   if (runtmp_d != flame_used_power_CHTotal) EEPROM.put(EpromPosition, flame_used_power_CHTotal);
   EpromPosition += sizeof(flame_used_power_CHTotal);
  }
  return retval;
}