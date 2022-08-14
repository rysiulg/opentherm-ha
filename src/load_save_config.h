

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

  if (dane.indexOf("COWATER_TOPIC") >= 0)    { tmpstrval = getJsonVal(dane, "COWATER_TOPIC"); tmpstrval.replace("\"",""); COWATER_TOPIC = tmpstrval; }
  if (dane.indexOf("COWATER_json") >= 0)       { tmpstrval = getJsonVal(dane, "COWATER_json"); tmpstrval.replace("\"",""); COWATER_json = tmpstrval; }


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
  #if defined enableMQTT || defined enableMQTTAsync
  retval += build_JSON_Payload(F("COPUMP_GET_TOPIC"), String(COPUMP_GET_TOPIC), false, "\"");
  retval += build_JSON_Payload(F("COPumpStatus_json"), String(COPumpStatus_json), false, "\"");
  retval += build_JSON_Payload(F("WaterPumpStatus_json"), String(WaterPumpStatus_json), false, "\"");
  retval += build_JSON_Payload(F("ROOMS_F1_GET_TOPIC"), String(ROOMS_F1_GET_TOPIC), false, "\"");
  retval += build_JSON_Payload(F("roomF1temp_json"), String(roomF1temp_json), false, "\"");
  retval += build_JSON_Payload(F("roomF1tempset_json"), String(roomF1tempset_json), false, "\"");
  retval += build_JSON_Payload(F("ROOMS_F2_GET_TOPIC"), String(ROOMS_F2_GET_TOPIC), false, "\"");
  retval += build_JSON_Payload(F("roomF2temp_json"), String(roomF2temp_json), false, "\"");
  retval += build_JSON_Payload(F("roomF2tempset_json"), String(roomF2tempset_json), false, "\"");
  #endif
  retval += build_JSON_Payload(F("flame_time_total"), String(flame_time_total), false, "\"");
  retval += build_JSON_Payload(F("flame_time_waterTotal"), String(flame_time_waterTotal), false, "\"");
  retval += build_JSON_Payload(F("flame_time_CHTotal"), String(flame_time_CHTotal), false, "\"");
  retval += build_JSON_Payload(F("flame_used_power_kwh"), String(flame_used_power_kwh), false, "\"");
  retval += build_JSON_Payload(F("flame_used_power_waterTotal"), String(flame_used_power_waterTotal), false, "\"");
  retval += build_JSON_Payload(F("flame_used_power_CHTotal"), String(flame_used_power_CHTotal), false, "\"");
  retval += build_JSON_Payload(F("heatingEnabled"), String(heatingEnabled), false, "\"");

  retval += build_JSON_Payload(F("enableHotWater"), String(enableHotWater), false, "\"");
  retval += build_JSON_Payload(F("automodeCO"), String(automodeCO), false, "\"");
  retval += build_JSON_Payload(F("ecoMode"), String(ecoMode), false, "\"");
  retval += build_JSON_Payload(F("tempBoilerSet"), String(tempBoilerSet), false, "\"");
  retval += build_JSON_Payload(F("cutOffTemp"), String(cutOffTemp), false, "\"");
  retval += build_JSON_Payload(F("dhwTarget"), String(dhwTarget), false, "\"");
  retval += build_JSON_Payload(F("histCWU"), String(histCWU), false, "\"");
  retval += build_JSON_Payload(F("histCO"), String(histCO), false, "\"");

  retval += build_JSON_Payload(F("COWATER_TOPIC"), String(COWATER_TOPIC), false, "\"");
  retval += build_JSON_Payload(F("COWATER_json"), String(COWATER_json), false, "\"");

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