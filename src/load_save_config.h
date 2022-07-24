#include <EEPROM.h>

// Where in EEPROM?
#define CONFIG_START sizeof(flame_used_power_kwh)*15

#define stat_ecoMode 0
#define stat_heatingEnabled 2
#define stat_enableHotWater 4
#define stat_automodeCO 8

// load whats in EEPROM in to the local CONFIGURATION if it is a valid setting
bool loadConfig() {
  // is it correct?
  EEPROM.begin(CONFIG_START);  //Size can be anywhere between 4 and 4096 bytes.
  int EpromPosition = 1;
  EEPROM.get(EpromPosition,CRTrunNumber);
  if (isnan(CRTrunNumber)) CRTrunNumber=0;
  CRTrunNumber++;
  EEPROM.put(EpromPosition, CRTrunNumber);
  EpromPosition += sizeof(CRTrunNumber);
  EEPROM.get(EpromPosition, flame_time_total);
  EpromPosition += sizeof(flame_time_total);
  EEPROM.get(EpromPosition, flame_used_power_kwh);
  EpromPosition += sizeof(flame_used_power_kwh);
  EEPROM.get(EpromPosition, flame_time_waterTotal);
  EpromPosition += sizeof(flame_time_waterTotal);
  EEPROM.get(EpromPosition, flame_used_power_waterTotal);
  EpromPosition += sizeof(flame_used_power_waterTotal);
  EEPROM.get(EpromPosition, flame_time_CHTotal);
  EpromPosition += sizeof(flame_time_CHTotal);
  EEPROM.get(EpromPosition, flame_used_power_CHTotal);
  EpromPosition += sizeof(flame_used_power_CHTotal);
  byte statusy = 0;
  if (isnan(statusy)) statusy = '0';
  EEPROM.get(EpromPosition, statusy);
  heatingEnabled = (statusy & stat_heatingEnabled);
  enableHotWater = (statusy & stat_enableHotWater);
  automodeCO = (statusy & stat_automodeCO);
  ecoMode = (statusy & stat_ecoMode);

  EpromPosition += sizeof(statusy);
  float tmpfloat = 0;
  EEPROM.get(EpromPosition, tmpfloat);
  if (!isnan(tmpfloat)) tempBoilerSet = tmpfloat;
  EpromPosition += sizeof(tempBoilerSet);
  EEPROM.get(EpromPosition, tmpfloat);
  if (!isnan(tmpfloat)) cutOffTemp = tmpfloat;
  EpromPosition += sizeof(cutOffTemp);
  EEPROM.get(EpromPosition, tmpfloat);
  if (!isnan(tmpfloat)) dhwTarget = tmpfloat;

  if (isnan(flame_used_power_kwh) || (flame_used_power_kwh + 1) == 0) flame_used_power_kwh = 0;
  if (isnan(flame_time_total) || (flame_time_total + 1) == 0) flame_time_total = 0;
  if (isnan(flame_time_waterTotal) || (flame_time_waterTotal + 1) == 0) flame_time_waterTotal = 0;
  if (isnan(flame_time_CHTotal) || (flame_time_CHTotal + 1) == 0) flame_time_CHTotal = 0;
  if (isnan(flame_used_power_waterTotal) || (flame_used_power_waterTotal + 1) == 0) flame_used_power_waterTotal = 0;
  if (isnan(flame_used_power_CHTotal) || (flame_used_power_CHTotal + 1) == 0) flame_used_power_CHTotal = 0;

  if(SPIFFS.exists(configfile)) {
    String dane = "\0";
    File file = SPIFFS.open(configfile,"r");
    while (file.available()) {
      dane += (file.readStringUntil('\n'));
    }
    file.close();

      sprintf(log_chars, "loaded: %s",String(dane).c_str());
      log_message(log_chars);
      String tmpstrval = "\0";
//this block is as param from html
      if (dane.indexOf("WebSocketlog") != -1)          { WebSocketlog = (bool) getJsonVal(dane, "WebSocketlog").toInt(); }
      if (dane.indexOf("debugSerial") != -1)           { debugSerial = (bool) getJsonVal(dane, "debugSerial").toInt(); }
      if (dane.indexOf("sendlogtomqtt") != -1)         { sendlogtomqtt = (bool) getJsonVal(dane, "sendlogtomqtt").toInt(); }
      if (dane.indexOf("SSID_Name") != -1)             { tmpstrval = getJsonVal(dane, "SSID_Name"); tmpstrval.replace("\"",""); strcpy(ssid, tmpstrval.c_str()); }
      if (dane.indexOf("SSID_PAssword") != -1)         { tmpstrval = getJsonVal(dane, "SSID_PAssword"); tmpstrval.replace("\"",""); strcpy(pass, tmpstrval.c_str()); }
      if (dane.indexOf("MQTT_servername") != -1)       { tmpstrval = getJsonVal(dane, "MQTT_servername"); tmpstrval.replace("\"",""); strcpy(mqtt_server, tmpstrval.c_str()); }
      if (dane.indexOf("MQTT_port_No") != -1)          { mqtt_port = (int) getJsonVal(dane, "MQTT_port_No").toInt(); }
      if (dane.indexOf("MQTT_username") != -1)         { tmpstrval = getJsonVal(dane, "MQTT_username"); tmpstrval.replace("\"",""); strcpy(mqtt_user, tmpstrval.c_str()); }
      if (dane.indexOf("MQTT_Password_data") != -1)    { tmpstrval = getJsonVal(dane, "MQTT_Password_data"); tmpstrval.replace("\"",""); strcpy(mqtt_password, tmpstrval.c_str()); }
      if (dane.indexOf("INFLUXDB_URL") != -1)          { tmpstrval = getJsonVal(dane, "INFLUXDB_URL"); tmpstrval.replace("\"",""); strcpy(influx_server, tmpstrval.c_str()); }
      if (dane.indexOf("INFLUXDB_DB_NAME") != -1)      { tmpstrval = getJsonVal(dane, "INFLUXDB_DB_NAME"); tmpstrval.replace("\"",""); strcpy(influx_database, tmpstrval.c_str()); }
      if (dane.indexOf("INFLUXDB_USER") != -1)         { tmpstrval = getJsonVal(dane, "INFLUXDB_USER"); tmpstrval.replace("\"",""); strcpy(influx_user, tmpstrval.c_str()); }
      if (dane.indexOf("INFLUXDB_PASSWORD") != -1)     { tmpstrval = getJsonVal(dane, "INFLUXDB_PASSWORD"); tmpstrval.replace("\"",""); strcpy(influx_password, tmpstrval.c_str()); }
      if (dane.indexOf("influx_measurments") != -1)    { tmpstrval = getJsonVal(dane, "influx_measurments"); tmpstrval.replace("\"",""); strcpy(influx_measurments, tmpstrval.c_str()); }
      if (dane.indexOf("NEWS_GET_TOPIC") != -1)        { tmpstrval = getJsonVal(dane, "NEWS_GET_TOPIC"); tmpstrval.replace("\"",""); NEWS_GET_TOPIC = tmpstrval; }
      if (dane.indexOf("NEWStemp_json") != -1)         { tmpstrval = getJsonVal(dane, "NEWStemp_json"); tmpstrval.replace("\"",""); NEWStemp_json = tmpstrval; }
      if (dane.indexOf("COPUMP_GET_TOPIC") != -1)      { tmpstrval = getJsonVal(dane, "COPUMP_GET_TOPIC"); tmpstrval.replace("\"",""); COPUMP_GET_TOPIC = tmpstrval; }
      if (dane.indexOf("COPumpStatus_json") != -1)     { tmpstrval = getJsonVal(dane, "COPumpStatus_json"); tmpstrval.replace("\"",""); COPumpStatus_json = tmpstrval; }
      if (dane.indexOf("WaterPumpStatus_json") != -1)  { tmpstrval = getJsonVal(dane, "WaterPumpStatus_json"); tmpstrval.replace("\"",""); WaterPumpStatus_json = tmpstrval; }
      if (dane.indexOf("ROOMS_F1_GET_TOPIC") != -1)    { tmpstrval = getJsonVal(dane, "ROOMS_F1_GET_TOPIC"); tmpstrval.replace("\"",""); ROOMS_F1_GET_TOPIC = tmpstrval; }
      if (dane.indexOf("roomF1temp_json") != -1)       { tmpstrval = getJsonVal(dane, "roomF1temp_json"); tmpstrval.replace("\"",""); roomF1temp_json = tmpstrval; }
      if (dane.indexOf("roomF1tempset_json") != -1)    { tmpstrval = getJsonVal(dane, "roomF1tempset_json"); tmpstrval.replace("\"",""); roomF1tempset_json = tmpstrval; }
      if (dane.indexOf("ROOMS_F2_GET_TOPIC") != -1)    { tmpstrval = getJsonVal(dane, "ROOMS_F2_GET_TOPIC"); tmpstrval.replace("\"",""); ROOMS_F2_GET_TOPIC = tmpstrval; }
      if (dane.indexOf("roomF2temp_json") != -1)       { tmpstrval = getJsonVal(dane, "roomF2temp_json"); tmpstrval.replace("\"",""); roomF2temp_json = tmpstrval; }
      if (dane.indexOf("roomF2tempset_json") != -1)    { tmpstrval = getJsonVal(dane, "roomF2tempset_json"); tmpstrval.replace("\"",""); roomF2tempset_json = tmpstrval; }
//this block is as param from html
//next is appender from function so i use in saveconfig this function to append rest
      unsigned long long tmpval = 0;
      char * pEnd;
      if (dane.indexOf("flame_time_total") != -1) {
        tmpval = strtoull (getJsonVal(dane, "flame_time_total").c_str(), &pEnd, 10);
        if (tmpval > flame_time_total) flame_time_total = tmpval; //if eprom is reset
      }
      if (dane.indexOf("flame_time_waterTotal") != -1) {
        tmpval = strtoull (getJsonVal(dane, "flame_time_waterTotal").c_str(), &pEnd, 10);
        if (tmpval > flame_time_waterTotal) flame_time_waterTotal = tmpval; //if eprom is reset
      }
      if (dane.indexOf("flame_time_CHTotal") != -1) {
        tmpval = strtoull(getJsonVal(dane, "flame_time_CHTotal").c_str(), &pEnd, 10);
        if (tmpval > flame_time_CHTotal) flame_time_CHTotal = tmpval; //if eprom is reset
      }
      double tmpdouble = 0;
      if (dane.indexOf("flame_used_power_kwh") != -1) {
        tmpdouble = getJsonVal(dane, "flame_used_power_kwh").toDouble();
        if (tmpdouble > flame_used_power_kwh) flame_used_power_kwh = tmpdouble; //if eprom is reset
      }
      if (dane.indexOf("flame_used_power_waterTotal") != -1) {
        tmpdouble = getJsonVal(dane, "flame_used_power_waterTotal").toDouble();
        if (tmpdouble > flame_used_power_waterTotal) flame_used_power_waterTotal = tmpdouble; //if eprom is reset
      }
      if (dane.indexOf("flame_used_power_CHTotal") != -1) {
        tmpdouble = getJsonVal(dane, "flame_used_power_CHTotal").toDouble();
        if (tmpdouble > flame_used_power_CHTotal) flame_used_power_CHTotal = tmpdouble; //if eprom is reset
      }

 //     if (dane.indexOf("heatingEnabled") != -1)        heatingEnabled = (bool) getJsonVal(dane, "heatingEnabled").toInt();
 //     if (dane.indexOf("enableHotWater") != -1)        enableHotWater = (bool) getJsonVal(dane, "enableHotWater").toInt();
 //     if (dane.indexOf("automodeCO") != -1)            automodeCO = (bool) getJsonVal(dane, "automodeCO").toInt();
 //     if (dane.indexOf("ecoMode") != -1)               ecoMode = (bool) getJsonVal(dane, "ecoMode").toInt();
      u_int CRT = 0;
      if (dane.indexOf("CRT") != -1) {
        CRT = (u_int) getJsonVal(dane, "CRT").toInt();
        if (CRT > CRTrunNumber) CRTrunNumber = CRT;
      }
    return true; // return 1 if config loaded
  }
  return false; // return 0 if config NOT loaded
}

// save the CONFIGURATION in to EEPROM
bool SaveConfig() {
  log_message((char*)F("Saving config...........................prepare "));
  double runtmp = 0, runtmp1 = 0, runtmp2 = 0;
  int EpromPosition = 1 + sizeof(CRTrunNumber);
  EEPROM.get(EpromPosition,runtmp);
  EpromPosition += sizeof(flame_time_total) + sizeof(flame_used_power_kwh);
  EEPROM.get(EpromPosition,runtmp1);
  EpromPosition += sizeof(flame_time_waterTotal) + sizeof(flame_used_power_waterTotal);
  EEPROM.get(EpromPosition,runtmp2);
  EpromPosition += sizeof(flame_time_CHTotal) + sizeof(flame_used_power_CHTotal);
  byte statusy = 0, statusytmp = 0;
  statusy += int(heatingEnabled)*stat_heatingEnabled;
  statusy += int(enableHotWater)*stat_enableHotWater;
  statusy += int(automodeCO)*stat_automodeCO;
  statusy += int(ecoMode)*stat_ecoMode;
  EEPROM.get(EpromPosition, statusytmp);
  float tempBoilerSettmp = 0, cutOffTemptmp = 0, dhwTargettmp = 0;
  EpromPosition += sizeof(statusy);
  EEPROM.get(EpromPosition, tempBoilerSettmp);
  EpromPosition += sizeof(tempBoilerSettmp);
  EEPROM.get(EpromPosition, cutOffTemptmp);
  EpromPosition += sizeof(cutOffTemptmp);
  EEPROM.get(EpromPosition, dhwTargettmp);

  EpromPosition = 1 + sizeof(CRTrunNumber);
  if (runtmp != flame_time_total || runtmp != flame_time_waterTotal || runtmp != flame_time_CHTotal || statusytmp != statusy || tempBoilerSettmp != tempBoilerSet || cutOffTemptmp != cutOffTemp || dhwTargettmp != dhwTarget) {
    log_message((char*)F("Saving config...........................Something changed "));
    EEPROM.put(EpromPosition, flame_time_total);
    EpromPosition += sizeof(flame_time_total);
    EEPROM.put(EpromPosition, flame_used_power_kwh);
    EpromPosition += sizeof(flame_used_power_kwh);
    EEPROM.put(EpromPosition, flame_time_waterTotal);
    EpromPosition += sizeof(flame_time_waterTotal);
    EEPROM.put(EpromPosition, flame_used_power_waterTotal);
    EpromPosition += sizeof(flame_used_power_waterTotal);
    EEPROM.put(EpromPosition, flame_time_CHTotal);
    EpromPosition += sizeof(flame_time_CHTotal);
    EEPROM.put(EpromPosition, flame_used_power_CHTotal);
    EpromPosition += sizeof(flame_used_power_CHTotal);

    EEPROM.put(EpromPosition, statusy);
    EpromPosition += sizeof(statusy);
    EEPROM.put(EpromPosition, tempBoilerSet);
    EpromPosition += sizeof(tempBoilerSet);
    EEPROM.put(EpromPosition, cutOffTemp);
    EpromPosition += sizeof(cutOffTemp);
    EEPROM.put(EpromPosition, dhwTarget);
    EpromPosition += sizeof(dhwTarget);

    String configSave = F("{\"config\":99");
//this block is as param from html
    configSave += F(",\"debugSerial\":");
    configSave += String(debugSerial?1:0);
    configSave += F(",\"WebSocketlog\":");
    configSave += String(WebSocketlog?1:0);
    configSave += F(",\"sendlogtomqtt\":");
    configSave += String(sendlogtomqtt?1:0);
    configSave += F(",\"SSID_Name\":\"");
    configSave += String(ssid);
    configSave += F("\"");
    configSave += F(",\"SSID_PAssword\":\"");
    configSave += String(pass);
    configSave += F("\"");
    configSave += F(",\"MQTT_servername\":\"");
    configSave += String(mqtt_server);
    configSave += F("\"");
    configSave += F(",\"MQTT_port_No\":\"");
    configSave += String(mqtt_port);
    configSave += F("\"");
    configSave += F(",\"MQTT_username\":\"");
    configSave += String(mqtt_user);
    configSave += F("\"");
    configSave += F(",\"MQTT_Password_data\":\"");
    configSave += String(mqtt_password);
    configSave += F("\"");
    configSave += F(",\"INFLUXDB_URL\":\"");
    configSave += String(influx_server);
    configSave += F("\"");
    configSave += F(",\"INFLUXDB_DB_NAME\":\"");
    configSave += String(influx_database);
    configSave += F("\"");
    configSave += F(",\"INFLUXDB_USER\":\"");
    configSave += String(influx_user);
    configSave += F("\"");
    configSave += F(",\"INFLUXDB_PASSWORD\":\"");
    configSave += String(influx_password);
    configSave += F("\"");
    configSave += F(",\"influx_measurments\":\"");
    configSave += String(influx_measurments);
    configSave += F("\"");
    configSave += F(",\"NEWS_GET_TOPIC\":\"");
    configSave += String(NEWS_GET_TOPIC);
    configSave += F("\"");
    configSave += F(",\"NEWStemp_json\":\"");
    configSave += String(NEWStemp_json);
    configSave += F("\"");
    configSave += F(",\"COPUMP_GET_TOPIC\":\"");
    configSave += String(COPUMP_GET_TOPIC);
    configSave += F("\"");
    configSave += F(",\"COPumpStatus_json\":\"");
    configSave += String(COPumpStatus_json);
    configSave += F("\"");
    configSave += F(",\"WaterPumpStatus_json\":\"");
    configSave += String(WaterPumpStatus_json);
    configSave += F("\"");
    configSave += F(",\"ROOMS_F1_GET_TOPIC\":\"");
    configSave += String(ROOMS_F1_GET_TOPIC);
    configSave += F("\"");
    configSave += F(",\"roomF1temp_json\":\"");
    configSave += String(roomF1temp_json);
    configSave += F("\"");
    configSave += F(",\"roomF1tempset_json\":\"");
    configSave += String(roomF1tempset_json);
    configSave += F("\"");
    configSave += F(",\"ROOMS_F2_GET_TOPIC\":\"");
    configSave += String(ROOMS_F2_GET_TOPIC);
    configSave += F("\"");
    configSave += F(",\"roomF2temp_json\":\"");
    configSave += String(roomF2temp_json);
    configSave += F("\"");
    configSave += F(",\"roomF2tempset_json\":\"");
    configSave += String(roomF2tempset_json);
    configSave += F("\"");

//this block is as param from html
//next is appender from function so i use in saveconfig this function to append rest
    configSave += addusage_local_values();
    configSave += F("}");
    if(SPIFFS.exists(configfile)) {SPIFFS.remove(configfile);}
    File file = SPIFFS.open(configfile, "w");
    if (!file){
      log_message((char*)F("Error opening file for write config..."));
      return false;
    }
    if (!file.println(configSave)) {
      log_message((char*)F("File was not written with config"));
      file.close();
      return false;
    } else {
      file.close();
    }
    return true;
  }
  return false;
}

String addusage_local_values()
{
  String retval = "\0";
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
  retval += ",\"CRT\":";
  retval += String(CRTrunNumber);

return retval;
}