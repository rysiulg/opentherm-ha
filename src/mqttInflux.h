

void updateInfluxDB()
{
#ifdef ENABLE_INFLUX
  log_message((char*)F("Sending data to InfluxDB ..."));
  String boilermode = Boiler_Mode();
  InfluxSensor.clearFields();
  // Report RSSI of currently connected network
  InfluxSensor.addField("rssi_BCO", (WiFi.RSSI()));
  InfluxSensor.addField("CRT_BCO", (runNumber));
  InfluxSensor.addField(String(ROOM_OTHERS_TEMPERATURE), roomtemp);
  InfluxSensor.addField(String(ROOM_OTHERS_TEMPERATURE_SETPOINT), roomtempSet);
  InfluxSensor.addField(String(ROOM_OTHERS_PRESSURE), pressure);
  InfluxSensor.addField(String(HOT_WATER_TEMPERATURE), tempCWU);
  InfluxSensor.addField(String(HOT_WATER_TEMPERATURE_SETPOINT), dhwTarget);

  InfluxSensor.addField(String(HOT_WATER_SOFTWARE_CH_STATE), enableHotWater ? 1 : 0);
  InfluxSensor.addField(String(HOT_WATER_CH_STATE), status_WaterActive ? 1 : 0);
  InfluxSensor.addField(String(BOILER_TEMPERATURE), tempBoiler);
  InfluxSensor.addField(String(BOILER_TEMPERATURE_SETPOINT), tempBoilerSet);
  InfluxSensor.addField(String(BOILER_TEMPERATURE_RET), retTemp);
  InfluxSensor.addField(String(BOILER_CH_STATE), status_CHActive ? 1 : 0);
  int boilermodewart = 0;
  if (boilermode == "auto")
    boilermodewart = 2;
  if (boilermode == "heat")
    boilermodewart = 1;
  InfluxSensor.addField(String(BOILER_SOFTWARE_CH_STATE_MODE), boilermodewart);
  InfluxSensor.addField(String(FLAME_STATE), status_FlameOn ? 1 : 0);
  InfluxSensor.addField(String(FLAME_LEVEL), flame_level);
  InfluxSensor.addField(String(FLAME_W), flame_used_power);
  InfluxSensor.addField(String(FLAME_W_TOTAL), flame_used_power_kwh);
  InfluxSensor.addField(String(FLAME_TIME_SEC_TOTAL), flame_time_total);
  InfluxSensor.addField(String(FLAME_W_DHW_TOTAL), flame_used_power_waterTotal);
  InfluxSensor.addField(String(FLAME_TIME_SEC_DHW_TOTAL), flame_time_waterTotal);
  InfluxSensor.addField(String(FLAME_W_CH_TOTAL), flame_used_power_CHTotal);
  InfluxSensor.addField(String(FLAME_TIME_SEC_CH_TOTAL), flame_time_CHTotal);


  InfluxSensor.addField(String(ECOMODE_STATE), ecoMode ? "1" : "0");

  InfluxSensor.addField(String(TEMP_CUTOFF), cutOffTemp);
  InfluxSensor.addField(String(DIAGS_OTHERS_FAULT), status_Fault ? "1" : "0");
  InfluxSensor.addField(String(DIAGS_OTHERS_DIAG), status_Diagnostic ? "1" : "0");
  InfluxSensor.addField(String(INTEGRAL_ERROR_GET_TOPIC), ierr);
  InfluxSensor.addField(String(LOG_GET_TOPIC), LastboilerResponseError);

  // Print what are we exactly writing
  sprintf(log_chars, "Writing to InfluxDB:  %s", String(InfluxClient.getLastErrorMessage()).c_str());
  log_message(log_chars);
  //WebSerial.println(InfluxClient.pointToLineProtocol(InfluxSensor));
  // Write point
  if (!InfluxClient.writePoint(InfluxSensor))
  {
    sprintf(log_chars, "InfluxDB write failed: %s", String(InfluxClient.getLastErrorMessage()).c_str());
    log_message(log_chars);
  }
#endif
}



void HADiscovery(String sensorswitchValTopic, String appendname, String nameval, String discoverytopic, String DeviceClass = "\0", String unitClass = "\0", String stateClass = "\0", String HAicon = "\0")
{
  const String deviceid = "\"dev\":{\"ids\":\""+String(me_lokalizacja)+"\",\"name\":\""+String(me_lokalizacja)+"\",\"sw\":\"" + String(version) + "\",\"mdl\": \""+String(me_lokalizacja)+"\",\"mf\":\"" + String(MFG) + "\"}";
  const String payloadvalue_startend_val = F(""); // value added before and after value send to mqtt queue
  const String payloadON = F("1");
  const String payloadOFF = F("0");

  String unitbuilder = "\0";
  int DCswitch = 0;
  #define tempswitch 1
  #define energyswitch 2
  DeviceClass.toLowerCase();
  stateClass.toLowerCase();
  HAicon.toLowerCase();
  if (unitClass.length() == 0) unitClass = " ";
  if (DeviceClass.length()>0)
  {
    unitbuilder += ",\"dev_cla\":\"" + DeviceClass + "\"";
    if (DeviceClass.indexOf("temperature") >= 0) DCswitch = tempswitch;
    if (DeviceClass.indexOf("energy") >= 0) DCswitch = energyswitch;
    switch (DCswitch) {
      case tempswitch: {
        if (unitClass.length() == 0 || unitClass == " ") unitClass = "°C";
        unitbuilder += ",\"unit_of_meas\":\"" + unitClass + "\"";
        if (stateClass.length()>0) unitbuilder += ",\"state_class\":\"" + stateClass + "\"";
        if (HAicon.length() == 0) HAicon = "mdi:thermometer";
        unitbuilder += ",\"ic\": \"" + HAicon + "\"";
      }
      case energyswitch: {
        if (unitClass.length() == 0 || unitClass == " ") unitClass = "kWh";
        unitbuilder += ",\"unit_of_meas\":\"" + unitClass + "\"";
        if (stateClass.length() == 0) stateClass = "total_increasing";
        unitbuilder += ",\"state_class\":\"" + stateClass + "\"";
        if (HAicon.length() == 0) HAicon = "mdi:fire";
        unitbuilder += ",\"ic\": \"" + HAicon + "\"";
      }
      default: {
        if (unitClass.length() > 0) unitbuilder += ",\"unit_of_meas\":\"" + unitClass + "\"";
        if (stateClass.length() > 0) unitbuilder += ",\"state_class\":\"" + stateClass + "\"";
        if (HAicon.length() > 0) unitbuilder += ",\"ic\": \"" + HAicon + "\"";
      }
    }

  }
  mqttclient.publish((discoverytopic + "_" + nameval + "/config").c_str(), ("{\"name\":\"" + appendname + nameval + "\",\"uniq_id\": \"" + appendname + nameval + "\",\"stat_t\":\"" + sensorswitchValTopic + "\",\"val_tpl\":\"{{value_json." + appendname + nameval + "}}\"" + unitbuilder + ",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
}


// This function  sends data to MQTT .
void updateMQTTData()
{
  log_message((char*)F("Sending data to MQTT ..."));
  const String deviceid = "\"dev\":{\"ids\":\""+String(me_lokalizacja)+"\",\"name\":\""+String(me_lokalizacja)+"\",\"sw\":\"" + String(version) + "\",\"mdl\": \""+String(me_lokalizacja)+"\",\"mf\":\"" + String(MFG) + "\"}";
  const String payloadvalue_startend_val = F(""); // value added before and after value send to mqtt queue
  const String payloadON = F("1");
  const String payloadOFF = F("0");

  mqttclient.publish(String(LOG_GET_TOPIC).c_str(), LastboilerResponseError.c_str());

  String boilermode = Boiler_Mode();

  if (status_Fault)
  { sprintf(log_chars, "Błąd: %s", String(status_Fault ? "on" : "off").c_str());
    log_message(log_chars);
  }
  if (status_CHActive)
  { sprintf(log_chars, "Status_CHActive: %s", String(status_CHActive ? "on" : "off").c_str());
    log_message(log_chars);
  }
  if (status_WaterActive)
  { sprintf(log_chars, "Status_WaterActive: %s", String(status_WaterActive ? "on" : "off").c_str());
    log_message(log_chars);
  }
  if (enableHotWater)
  { sprintf(log_chars, "EnableHW: %s", String(enableHotWater ? "on" : "off").c_str());
    log_message(log_chars);
  }
  if (status_FlameOn)
  { sprintf(log_chars, "Status_FlameOn: %s", String(status_FlameOn ? "on" : "off").c_str());
    log_message(log_chars);
  }
  if (status_Cooling)
  { sprintf(log_chars, "Status_Cooling: %s", String(status_Cooling ? "on" : "off").c_str());
    log_message(log_chars);
  }
  if (status_Diagnostic)
  { sprintf(log_chars, "Status_Diagnostic: %s", String(status_Diagnostic ? "on" : "off").c_str());
    log_message(log_chars);
  }

  mqttclient.publish(String(ROOM_OTHERS_TOPIC).c_str(),(
                      "{\"" + String(OT) + (ROOM_OTHERS_TEMPERATURE) + "\": " + payloadvalue_startend_val + String(roomtemp) + payloadvalue_startend_val +
                      ",\"" + String(OT) + (ROOM_OTHERS_TEMPERATURE_SETPOINT) + "\": " + payloadvalue_startend_val + String(roomtempSet) + payloadvalue_startend_val +
                      ",\"" + String(OT) + (ROOM_OTHERS_PRESSURE) + "\": " + payloadvalue_startend_val + String(pressure) + payloadvalue_startend_val +
                      "}").c_str(), mqtt_Retain); //"heat" : "off")
#ifdef debug
log_message((char*)F("mqtt publish 1st end"));
#endif
  mqttclient.publish(String(HOT_WATER_TOPIC).c_str(),
                     ("{\"" + String(OT) + String(HOT_WATER_TEMPERATURE) + "\": " + payloadvalue_startend_val + String(tempCWU) + payloadvalue_startend_val +
                      ",\"" + String(OT) + String(HOT_WATER_TEMPERATURE_SETPOINT) + "\": " + payloadvalue_startend_val + String(dhwTarget, 1) + payloadvalue_startend_val +
                      ",\"" + String(OT) + String(HOT_WATER_CH_STATE) + "\": " + payloadvalue_startend_val + String(status_WaterActive ? payloadON : payloadOFF) + payloadvalue_startend_val +
                      ",\"" + String(OT) + String(HOT_WATER_SOFTWARE_CH_STATE) + "\": \"" + String(enableHotWater ? "heat" : "off") + "\"" +
                      ",\"" + String(OT) + String(FLAME_W_DHW_TOTAL) + "\": " + payloadvalue_startend_val + String(flame_used_power_waterTotal, 4) + payloadvalue_startend_val +
                      ",\"" + String(OT) + String(FLAME_TIME_SEC_DHW_TOTAL) + "\": " + payloadvalue_startend_val + String(flame_time_waterTotal) + payloadvalue_startend_val +
                      "}").c_str(), mqtt_Retain); //"heat" : "off")
#ifdef debug
log_message((char*)F("mqtt publish 2nd end"));
#endif
  mqttclient.publish(String(BOILER_TOPIC).c_str(),
                     ("{\"" + String(OT) + String(BOILER_TEMPERATURE) + "\": " + payloadvalue_startend_val + String(tempBoiler) + payloadvalue_startend_val +
                      ",\"" + String(OT) + String(BOILER_TEMPERATURE_RET) + "\": " + payloadvalue_startend_val + String(retTemp) + payloadvalue_startend_val +
                      ",\"" + String(OT) + String(BOILER_TEMPERATURE_SETPOINT) + "\": " + payloadvalue_startend_val + String(tempBoilerSet, 1) + payloadvalue_startend_val +
                      ",\"" + String(OT) + String(BOILER_CH_STATE) + "\": " + payloadvalue_startend_val + String(status_CHActive ? payloadON : payloadOFF) + payloadvalue_startend_val +
                      ",\"" + String(OT) + String(ECOMODE_STATE) + "\": " + payloadvalue_startend_val + String(ecoMode ? payloadON : payloadOFF) + payloadvalue_startend_val +
                      ",\"" + String(OT) + String(BOILER_SOFTWARE_CH_STATE_MODE) + "\": \"" + String(boilermode) + "\""+
                      ",\"" + String(OT) + String(FLAME_STATE) + "\": " + payloadvalue_startend_val + String(status_FlameOn ? payloadON : payloadOFF) + payloadvalue_startend_val +
                      ",\"" + String(OT) + String(FLAME_LEVEL) + "\": " + payloadvalue_startend_val + String(flame_level, 0) + payloadvalue_startend_val +
                      ",\"" + String(OT) + String(FLAME_W) + "\": " + payloadvalue_startend_val + String(flame_used_power, 4) + payloadvalue_startend_val +
                      ",\"" + String(OT) + String(FLAME_W_TOTAL) + "\": " + payloadvalue_startend_val + String(flame_used_power_kwh, 4) + payloadvalue_startend_val +
                      ",\"" + String(OT) + String(FLAME_TIME_SEC_TOTAL) + "\": " + payloadvalue_startend_val + String(flame_time_total) + payloadvalue_startend_val +
                      ",\"" + String(OT) + String(FLAME_W_CH_TOTAL) + "\": " + payloadvalue_startend_val + String(flame_used_power_CHTotal, 4) + payloadvalue_startend_val +
                      ",\"" + String(OT) + String(FLAME_TIME_SEC_CH_TOTAL) + "\": " + payloadvalue_startend_val + String(flame_time_CHTotal) + payloadvalue_startend_val +
                      ",\"" + String(OT) + String(TEMP_CUTOFF) + "\": " + payloadvalue_startend_val + String(cutOffTemp, 1) + payloadvalue_startend_val +
                      "}").c_str(), mqtt_Retain); //"heat" : "off")    boilermode.c_str(),1);// ? "auto" : "heat" : "off",1); //heatingEnabled ? "1" : "0",1);  //"heat" : "off",1);

  mqttclient.publish(String(DIAG_TOPIC).c_str(),
                     ("{\"" + String(OT) + String(DIAGS_OTHERS_FAULT) + "\": " + payloadvalue_startend_val + String(status_Fault ? payloadON : payloadOFF) + payloadvalue_startend_val +
                      ",\"" + String(OT) + String(DIAGS_OTHERS_DIAG) + "\": " + payloadvalue_startend_val + String(status_Diagnostic ? payloadON : payloadOFF) + payloadvalue_startend_val +
                      ",\"" + String(OT) + String(INTEGRAL_ERROR_GET_TOPIC) + "\": " + payloadvalue_startend_val + String(ierr) + payloadvalue_startend_val +
                      "}").c_str(), mqtt_Retain); //"heat" : "off")


  publishhomeassistantconfig++; // zwiekszamy licznik wykonan wyslania mqtt by co publishhomeassistantconfigdivider wysłań wysłać autoconfig discovery dla homeassisatnt
  if (publishhomeassistantconfig % publishhomeassistantconfigdivider == 0)
  {
    #ifdef debug
    log_message((char*)F("mqtt publish HomeAssistant start"));
    #endif
    //mqttclient.setBufferSize(2048);

    HADiscovery(String(DIAG_TOPIC), String(OT), String(DIAGS_OTHERS_FAULT), String(DIAG_HABS_TOPIC), "problem");
    HADiscovery(String(DIAG_TOPIC), String(OT), String(DIAGS_OTHERS_DIAG), String(DIAG_HABS_TOPIC));
    HADiscovery(String(DIAG_TOPIC), String(OT), String(INTEGRAL_ERROR_GET_TOPIC), String(DIAG_HA_TOPIC));
    HADiscovery(String(LOG_GET_TOPIC), String(OT), String(LOGS), String(DIAG_HA_TOPIC));
    HADiscovery(String(ROOM_OTHERS_TOPIC), String(OT), String(ROOM_OTHERS_TEMPERATURE), String(ROOM_OTHERS_HA_TOPIC), "temperature");
    HADiscovery(String(ROOM_OTHERS_TOPIC), String(OT), String(ROOM_OTHERS_TEMPERATURE_SETPOINT), String(ROOM_OTHERS_HA_TOPIC), "temperature");
    HADiscovery(String(ROOM_OTHERS_TOPIC), String(OT), String(ROOM_OTHERS_PRESSURE), String(ROOM_OTHERS_HA_TOPIC), "pressure", "hPa");
    HADiscovery(String(HOT_WATER_TOPIC), String(OT), String(HOT_WATER_TEMPERATURE), String(HOT_WATER_HA_TOPIC), "temperature");
    HADiscovery(String(HOT_WATER_TOPIC), String(OT), String(HOT_WATER_TEMPERATURE_SETPOINT), String(HOT_WATER_HA_TOPIC), "temperature");
    HADiscovery(String(HOT_WATER_TOPIC), String(OT), String(HOT_WATER_CH_STATE), String(HOT_WATER_HABS_TOPIC), "heat");
    HADiscovery(String(HOT_WATER_TOPIC), String(OT), String(HOT_WATER_SOFTWARE_CH_STATE), String(HOT_WATER_HABS_TOPIC), "heat");
    HADiscovery(String(BOILER_TOPIC), String(OT), String(BOILER_TEMPERATURE), String(BOILER_HA_TOPIC), "temperature");
    HADiscovery(String(BOILER_TOPIC), String(OT), String(BOILER_TEMPERATURE_RET), String(BOILER_HA_TOPIC), "temperature");
    HADiscovery(String(BOILER_TOPIC), String(OT), String(BOILER_TEMPERATURE_SETPOINT), String(BOILER_HA_TOPIC), "temperature");
    HADiscovery(String(BOILER_TOPIC), String(OT), String(BOILER_CH_STATE), String(BOILER_HABS_TOPIC), "heat");
    HADiscovery(String(BOILER_TOPIC), String(OT), String(ECOMODE_STATE), String(BOILER_HABS_TOPIC), "heat");
    HADiscovery(String(BOILER_TOPIC), String(OT), String(BOILER_SOFTWARE_CH_STATE_MODE), String(BOILER_HA_TOPIC));
    HADiscovery(String(BOILER_TOPIC), String(OT), String(FLAME_STATE), String(BOILER_HABS_TOPIC), "heat", "\0", "\0", "mdi:fire");
    HADiscovery(String(BOILER_TOPIC), String(OT), String(FLAME_LEVEL), String(BOILER_HA_TOPIC), "power", "%", "\0", "mdi:fire");
    HADiscovery(String(BOILER_TOPIC), String(OT), String(FLAME_W), String(BOILER_HA_TOPIC), "energy", "kWh", "total_increasing", "mdi:fire");
    HADiscovery(String(BOILER_TOPIC), String(OT), String(FLAME_W_TOTAL), String(BOILER_HA_TOPIC), "energy", "kWh", "total_increasing", "mdi:fire");
    HADiscovery(String(BOILER_TOPIC), String(OT), String(FLAME_TIME_SEC_TOTAL), String(BOILER_HA_TOPIC), "none", "s", "total_increasing", "mdi:fire");
    HADiscovery(String(BOILER_TOPIC), String(OT), String(FLAME_TIME_SEC_CH_TOTAL), String(BOILER_HA_TOPIC), "none", "s", "total_increasing", "mdi:fire");
    HADiscovery(String(BOILER_TOPIC), String(OT), String(FLAME_W_CH_TOTAL), String(BOILER_HA_TOPIC), "energy", "kWh", "total_increasing", "mdi:fire");
    HADiscovery(String(HOT_WATER_TOPIC), String(OT), String(FLAME_TIME_SEC_DHW_TOTAL), String(HOT_WATER_HA_TOPIC), "none", "s", "total_increasing", "mdi:fire");
    HADiscovery(String(HOT_WATER_TOPIC), String(OT), String(FLAME_W_DHW_TOTAL), String(HOT_WATER_HA_TOPIC), "energy", "kWh", "total_increasing", "mdi:fire");
    HADiscovery(String(BOILER_TOPIC), String(OT), String(TEMP_CUTOFF), String(BOILER_HA_TOPIC), "temperature");
    // homeassistant/sensor/BB050B_OPENTHERM_OT10_lo/config = {"name":"Opentherm OPENTHERM OT10 lo","stat_t":"tele/tasmota_BB050B/SENSOR","avty_t":"tele/tasmota_BB050B/LWT","pl_avail":"Online","pl_not_avail":"Offline","uniq_id":"BB050B_OPENTHERM_OT10_lo","dev":{"ids":["BB050B"]},"unit_of_meas":" ","ic":"mdi:eye","frc_upd":true,"val_tpl":"{{value_json['OPENTHERM']['OT10']['lo']}}"} (retained) problem
    // 21:16:02.724 MQT: homeassistant/sensor/BB050B_OPENTHERM_OT10_hi/config = {"name":"Opentherm OPENTHERM OT10 hi","stat_t":"tele/tasmota_BB050B/SENSOR","avty_t":"tele/tasmota_BB050B/LWT","pl_avail":"Online","pl_not_avail":"Offline","uniq_id":"BB050B_OPENTHERM_OT10_hi","dev":{"ids":["BB050B"]},"unit_of_meas":" ","ic":"mdi:eye","frc_upd":true,"val_tpl":"{{value_json['OPENTHERM']['OT10']['hi']}}"} (retained)
//    HADiscovery(String(DIAG_HABS_TOPIC), String(DIAGS_OTHERS_FAULT), String(OT), String(DIAG_TOPIC), "problem");
//    mqttclient.publish((String(DIAG_HABS_TOPIC) + "_" + String(DIAGS_OTHERS_FAULT) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(DIAGS_OTHERS_FAULT) + "\",\"uniq_id\": \"" + String(OT) + String(DIAGS_OTHERS_FAULT) + "\",\"stat_t\":\"" + String(DIAG_TOPIC) + "\",\"payload_on\": " + payloadON + ",\"payload_off\": " + payloadOFF + ",\"val_tpl\":\"{{value_json." + String(OT) + String(DIAGS_OTHERS_FAULT) + "}}\",\"dev_cla\":\"problem\",\"unit_of_meas\": \" \",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
//    HADiscovery(String(DIAG_HABS_TOPIC), String(DIAGS_OTHERS_DIAG), String(OT), String(DIAG_TOPIC));
    //mqttclient.publish((String(DIAG_HABS_TOPIC) + "_" + String(DIAGS_OTHERS_DIAG) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(DIAGS_OTHERS_DIAG) + "\",\"uniq_id\": \"" + String(OT) + String(DIAGS_OTHERS_DIAG) + "\",\"stat_t\":\"" + String(DIAG_TOPIC) + "\",\"payload_on\": " + payloadON + ",\"payload_off\": " + payloadOFF + ",\"val_tpl\":\"{{value_json." + String(OT) + String(DIAGS_OTHERS_DIAG) + "}}\",\"unit_of_meas\": \" \",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
//    HADiscovery(String(DIAG_HA_TOPIC), String(INTEGRAL_ERROR_GET_TOPIC), String(OT), String(DIAG_TOPIC));
    //mqttclient.publish((String(DIAG_HA_TOPIC) + "_" + String(INTEGRAL_ERROR_GET_TOPIC) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(INTEGRAL_ERROR_GET_TOPIC) + "\",\"uniq_id\": \"" + String(OT) + String(INTEGRAL_ERROR_GET_TOPIC) + "\",\"stat_t\":\"" + String(DIAG_TOPIC) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(INTEGRAL_ERROR_GET_TOPIC) + "}}\",\"unit_of_meas\": \" \",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
//    HADiscovery(String(DIAG_HA_TOPIC), String(LOGS), String(OT), String(LOG_GET_TOPIC));
//    mqttclient.publish((String(DIAG_HA_TOPIC) + "_" + String(LOGS) + "/config").c_str(), ("{\"name\":\"" + String(OT) + LOGS + "\",\"uniq_id\": \"" + String(OT) + String(LOGS) + "\",\"stat_t\":\"" + String(LOG_GET_TOPIC) + "\",\"val_tpl\":\"{{ value }}\",\"unit_of_meas\": \" \",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);

//    HADiscovery(String(ROOM_OTHERS_HA_TOPIC), String(ROOM_OTHERS_TEMPERATURE), String(OT), String(ROOM_OTHERS_TOPIC), "temperature");
//    mqttclient.publish((String(ROOM_OTHERS_HA_TOPIC) + "_" + String(ROOM_OTHERS_TEMPERATURE) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(ROOM_OTHERS_TEMPERATURE) + "\",\"uniq_id\": \"" + String(OT) + String(ROOM_OTHERS_TEMPERATURE) + "\",\"stat_t\":\"" + String(ROOM_OTHERS_TOPIC) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(ROOM_OTHERS_TEMPERATURE) + "}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
//    HADiscovery(String(ROOM_OTHERS_HA_TOPIC), String(ROOM_OTHERS_TEMPERATURE_SETPOINT), String(OT), String(ROOM_OTHERS_TOPIC), "temperature");
//    mqttclient.publish((String(ROOM_OTHERS_HA_TOPIC) + "_" + String(ROOM_OTHERS_TEMPERATURE_SETPOINT) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(ROOM_OTHERS_TEMPERATURE_SETPOINT) + "\",\"uniq_id\": \"" + String(OT) + String(ROOM_OTHERS_TEMPERATURE_SETPOINT) + "\",\"stat_t\":\"" + String(ROOM_OTHERS_TOPIC) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(ROOM_OTHERS_TEMPERATURE_SETPOINT) + "}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
//    HADiscovery(String(ROOM_OTHERS_HA_TOPIC), String(ROOM_OTHERS_PRESSURE), String(OT), String(ROOM_OTHERS_TOPIC), "pressure", "hPa");
//    mqttclient.publish((String(ROOM_OTHERS_HA_TOPIC) + "_" + String(ROOM_OTHERS_PRESSURE) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(ROOM_OTHERS_PRESSURE) + "\",\"uniq_id\": \"" + String(OT) + String(ROOM_OTHERS_PRESSURE) + "\",\"stat_t\":\"" + String(ROOM_OTHERS_TOPIC) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(ROOM_OTHERS_PRESSURE) + "}}\",\"dev_cla\":\"pressure\",\"unit_of_meas\": \"hPa\",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);

//    HADiscovery(String(HOT_WATER_HA_TOPIC), String(HOT_WATER_TEMPERATURE), String(OT), String(HOT_WATER_TOPIC), "temperature");
//    mqttclient.publish((String(HOT_WATER_HA_TOPIC) + "_" + String(HOT_WATER_TEMPERATURE) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(HOT_WATER_TEMPERATURE) + "\",\"uniq_id\": \"" + String(OT) + String(HOT_WATER_TEMPERATURE) + "\",\"stat_t\":\"" + String(HOT_WATER_TOPIC) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(HOT_WATER_TEMPERATURE) + "}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
//    HADiscovery(String(HOT_WATER_HA_TOPIC), String(HOT_WATER_TEMPERATURE_SETPOINT), String(OT), String(HOT_WATER_TOPIC), "temperature");
//    mqttclient.publish((String(HOT_WATER_HA_TOPIC) + "_" + String(HOT_WATER_TEMPERATURE_SETPOINT) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(HOT_WATER_TEMPERATURE_SETPOINT) + "\",\"uniq_id\": \"" + String(OT) + String(HOT_WATER_TEMPERATURE_SETPOINT) + "\",\"stat_t\":\"" + String(HOT_WATER_TOPIC) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(HOT_WATER_TEMPERATURE_SETPOINT) + "}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
//    HADiscovery(String(HOT_WATER_HABS_TOPIC), String(HOT_WATER_CH_STATE), String(OT), String(HOT_WATER_TOPIC), "heat");
//    mqttclient.publish((String(HOT_WATER_HABS_TOPIC) + "_" + String(HOT_WATER_CH_STATE) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(HOT_WATER_CH_STATE) + "\",\"uniq_id\": \"" + String(OT) + String(HOT_WATER_CH_STATE) + "\",\"stat_t\":\"" + String(HOT_WATER_TOPIC) + "\",\"payload_on\": " + payloadON + ",\"payload_off\": " + payloadOFF + ",\"val_tpl\":\"{{value_json." + String(OT) + String(HOT_WATER_CH_STATE) + "}}\",\"dev_cla\":\"heat\",\"unit_of_meas\":\" \",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
//    HADiscovery(String(HOT_WATER_HABS_TOPIC), String(HOT_WATER_SOFTWARE_CH_STATE), String(OT), String(HOT_WATER_TOPIC), "heat");
//    mqttclient.publish((String(HOT_WATER_HABS_TOPIC) + "_" + String(HOT_WATER_SOFTWARE_CH_STATE) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(HOT_WATER_SOFTWARE_CH_STATE) + "\",\"uniq_id\": \"" + String(OT) + String(HOT_WATER_SOFTWARE_CH_STATE) + "\",\"stat_t\":\"" + String(HOT_WATER_TOPIC) + "\",\"payload_on\": " + payloadON + ",\"payload_off\": " + payloadOFF + ",\"val_tpl\":\"{{value_json." + String(OT) + String(HOT_WATER_SOFTWARE_CH_STATE) + "}}\",\"dev_cla\":\"heat\",\"unit_of_meas\": \" \",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);

//    HADiscovery(String(BOILER_HA_TOPIC), String(BOILER_TEMPERATURE), String(OT), String(BOILER_TOPIC), "temperature");
//    mqttclient.publish((String(BOILER_HA_TOPIC) + "_" + String(BOILER_TEMPERATURE) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(BOILER_TEMPERATURE) + "\",\"uniq_id\": \"" + String(OT) + String(BOILER_TEMPERATURE) + "\",\"stat_t\":\"" + String(BOILER_TOPIC) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(BOILER_TEMPERATURE) + "}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
//    HADiscovery(String(BOILER_HA_TOPIC), String(BOILER_TEMPERATURE_RET), String(OT), String(BOILER_TOPIC), "temperature");
//    mqttclient.publish((String(BOILER_HA_TOPIC) + "_" + String(BOILER_TEMPERATURE_RET) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(BOILER_TEMPERATURE_RET) + "\",\"uniq_id\": \"" + String(OT) + String(BOILER_TEMPERATURE_RET) + "\",\"stat_t\":\"" + String(BOILER_TOPIC) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(BOILER_TEMPERATURE_RET) + "}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
//    HADiscovery(String(BOILER_HA_TOPIC), String(BOILER_TEMPERATURE_SETPOINT), String(OT), String(BOILER_TOPIC), "temperature");
//    mqttclient.publish((String(BOILER_HA_TOPIC) + "_" + String(BOILER_TEMPERATURE_SETPOINT) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(BOILER_TEMPERATURE_SETPOINT) + "\",\"uniq_id\": \"" + String(OT) + String(BOILER_TEMPERATURE_SETPOINT) + "\",\"stat_t\":\"" + String(BOILER_TOPIC) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(BOILER_TEMPERATURE_SETPOINT) + "}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
//    HADiscovery(String(BOILER_HABS_TOPIC), String(BOILER_CH_STATE), String(OT), String(BOILER_TOPIC), "heat");
//    mqttclient.publish((String(BOILER_HABS_TOPIC) + "_" + String(BOILER_CH_STATE) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(BOILER_CH_STATE) + "\",\"uniq_id\": \"" + String(OT) + String(BOILER_CH_STATE) + "\",\"stat_t\":\"" + String(BOILER_TOPIC) + "\",\"payload_on\": " + payloadON + ",\"payload_off\": " + payloadOFF + ",\"val_tpl\":\"{{value_json." + String(OT) + String(BOILER_CH_STATE) + "}}\",\"dev_cla\":\"heat\",\"unit_of_meas\": \" \",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
//    HADiscovery(String(BOILER_HABS_TOPIC), String(ECOMODE_STATE), String(OT), String(BOILER_TOPIC), "heat");
//    mqttclient.publish((String(BOILER_HABS_TOPIC) + "_" + String(ECOMODE_STATE) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(ECOMODE_STATE) + "\",\"uniq_id\": \"" + String(OT) + String(ECOMODE_STATE) + "\",\"stat_t\":\"" + String(BOILER_TOPIC) + "\",\"payload_on\": " + payloadON + ",\"payload_off\": " + payloadOFF + ",\"val_tpl\":\"{{value_json." + String(OT) + String(ECOMODE_STATE) + "}}\",\"dev_cla\":\"heat\",\"unit_of_meas\": \" \",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
//    HADiscovery(String(BOILER_HA_TOPIC), String(BOILER_SOFTWARE_CH_STATE_MODE), String(OT), String(BOILER_TOPIC));
//    mqttclient.publish((String(BOILER_HA_TOPIC) + "_" + String(BOILER_SOFTWARE_CH_STATE_MODE) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(BOILER_SOFTWARE_CH_STATE_MODE) + "\",\"uniq_id\": \"" + String(OT) + String(BOILER_SOFTWARE_CH_STATE_MODE) + "\",\"stat_t\":\"" + String(BOILER_TOPIC) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(BOILER_SOFTWARE_CH_STATE_MODE) + "}}\",\"unit_of_meas\": \" \",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
//    HADiscovery(String(BOILER_HABS_TOPIC), String(FLAME_STATE), String(OT), String(BOILER_TOPIC), "heat", "\0", "\0", "mdi:fire");
//    mqttclient.publish((String(BOILER_HABS_TOPIC) + "_" + String(FLAME_STATE) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(FLAME_STATE) + "\",\"uniq_id\": \"" + String(OT) + String(FLAME_STATE) + "\",\"stat_t\":\"" + String(BOILER_TOPIC) + "\",\"payload_on\": " + payloadON + ",\"payload_off\": " + payloadOFF + ",\"val_tpl\":\"{{value_json." + String(OT) + String(FLAME_STATE) + "}}\",\"dev_cla\":\"heat\",\"unit_of_meas\":\" \",\"ic\": \"mdi:fire\",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
//    HADiscovery(String(BOILER_HA_TOPIC), String(FLAME_LEVEL), String(OT), String(BOILER_TOPIC), "power", "%", "\0", "mdi:fire");
//    mqttclient.publish((String(BOILER_HA_TOPIC) + "_" + String(FLAME_LEVEL) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(FLAME_LEVEL) + "\",\"uniq_id\": \"" + String(OT) + String(FLAME_LEVEL) + "\",\"stat_t\":\"" + String(BOILER_TOPIC) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(FLAME_LEVEL) + "}}\",\"dev_cla\":\"power\",\"unit_of_meas\":\"%\",\"ic\": \"mdi:fire\",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
//    HADiscovery(String(BOILER_HA_TOPIC), String(FLAME_W), String(OT), String(BOILER_TOPIC), "energy", "kWh", "total_increasing", "mdi:fire");
//    mqttclient.publish((String(BOILER_HA_TOPIC) + "_" + String(FLAME_W) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(FLAME_W) + "\",\"uniq_id\": \"" + String(OT) + String(FLAME_W) + "\",\"stat_t\":\"" + String(BOILER_TOPIC) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(FLAME_W) + "}}\",\"dev_cla\":\"energy\",\"unit_of_meas\":\"kWh\",\"state_class\":\"measurement\",\"ic\": \"mdi:fire\",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
//    HADiscovery(String(BOILER_HA_TOPIC), String(FLAME_W_TOTAL), String(OT), String(BOILER_TOPIC), "energy", "kWh", "total_increasing", "mdi:fire");
//    mqttclient.publish((String(BOILER_HA_TOPIC) + "_" + String(FLAME_W_TOTAL) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(FLAME_W_TOTAL) + "\",\"uniq_id\": \"" + String(OT) + String(FLAME_W_TOTAL) + "\",\"stat_t\":\"" + String(BOILER_TOPIC) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(FLAME_W_TOTAL) + "}}\",\"dev_cla\":\"energy\",\"unit_of_meas\":\"kWh\",\"state_class\":\"total_increasing\",\"ic\": \"mdi:fire\",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);

//    HADiscovery(String(BOILER_HA_TOPIC), String(FLAME_TIME_SEC_TOTAL), String(OT), String(BOILER_TOPIC), "none", "s", "total_increasing", "mdi:fire");
//    mqttclient.publish((String(BOILER_HA_TOPIC) + "_" + String(FLAME_TIME_SEC_TOTAL) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(FLAME_TIME_SEC_TOTAL) + "\",\"uniq_id\": \"" + String(OT) + String(FLAME_TIME_SEC_TOTAL) + "\",\"stat_t\":\"" + String(BOILER_TOPIC) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(FLAME_TIME_SEC_TOTAL) + "}}\",\"dev_cla\":\"none\",\"unit_of_meas\":\"s\",\"state_class\":\"total_increasing\",\"ic\": \"mdi:fire\",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
//    HADiscovery(String(BOILER_HA_TOPIC), String(FLAME_TIME_SEC_CH_TOTAL), String(OT), String(BOILER_TOPIC), "none", "s", "total_increasing", "mdi:fire");
//    mqttclient.publish((String(BOILER_HA_TOPIC) + "_" + String(FLAME_TIME_SEC_CH_TOTAL) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(FLAME_TIME_SEC_CH_TOTAL) + "\",\"uniq_id\": \"" + String(OT) + String(FLAME_TIME_SEC_CH_TOTAL) + "\",\"stat_t\":\"" + String(BOILER_TOPIC) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(FLAME_TIME_SEC_CH_TOTAL) + "}}\",\"unit_of_meas\":\"s\",\"state_class\":\"measurement\",\"ic\": \"mdi:fire\",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
//    HADiscovery(String(BOILER_HA_TOPIC), String(FLAME_W_CH_TOTAL), String(OT), String(BOILER_TOPIC), "energy", "kWh", "total_increasing", "mdi:fire");
//    mqttclient.publish((String(BOILER_HA_TOPIC) + "_" + String(FLAME_W_CH_TOTAL) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(FLAME_W_CH_TOTAL) + "\",\"uniq_id\": \"" + String(OT) + String(FLAME_W_CH_TOTAL) + "\",\"stat_t\":\"" + String(BOILER_TOPIC) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(FLAME_W_CH_TOTAL) + "}}\",\"dev_cla\":\"energy\",\"unit_of_meas\":\"kWh\",\"state_class\":\"total_increasing\",\"ic\": \"mdi:fire\",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);

//    HADiscovery(String(HOT_WATER_HA_TOPIC), String(FLAME_TIME_SEC_DHW_TOTAL), String(OT), String(HOT_WATER_TOPIC), "none", "s", "total_increasing", "mdi:fire");
    //mqttclient.publish((String(HOT_WATER_HA_TOPIC) + "_" + String(FLAME_TIME_SEC_DHW_TOTAL) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(FLAME_TIME_SEC_DHW_TOTAL) + "\",\"uniq_id\": \"" + String(OT) + String(FLAME_TIME_SEC_DHW_TOTAL) + "\",\"stat_t\":\"" + String(HOT_WATER_TOPIC) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(FLAME_TIME_SEC_DHW_TOTAL) + "}}\",\"unit_of_meas\":\"s\",\"state_class\":\"measurement\",\"ic\": \"mdi:fire\",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
//    HADiscovery(String(HOT_WATER_HA_TOPIC), String(FLAME_W_DHW_TOTAL), String(OT), String(HOT_WATER_TOPIC), "energy", "kWh", "total_increasing", "mdi:fire");
//    mqttclient.publish((String(HOT_WATER_HA_TOPIC) + "_" + String(FLAME_W_DHW_TOTAL) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(FLAME_W_DHW_TOTAL) + "\",\"uniq_id\": \"" + String(OT) + String(FLAME_W_DHW_TOTAL) + "\",\"stat_t\":\"" + String(HOT_WATER_TOPIC) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(FLAME_W_DHW_TOTAL) + "}}\",\"dev_cla\":\"energy\",\"unit_of_meas\":\"kWh\",\"state_class\":\"total_increasing\",\"ic\": \"mdi:fire\",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
//    HADiscovery(String(BOILER_HA_TOPIC), String(TEMP_CUTOFF), String(OT), String(BOILER_TOPIC), "temperature");
//    mqttclient.publish((String(BOILER_HA_TOPIC) + "_" + String(TEMP_CUTOFF) + "/config").c_str(), ("{\"name\":\"" + String(OT) + String(TEMP_CUTOFF) + "\",\"uniq_id\": \"" + String(OT) + String(TEMP_CUTOFF) + "\",\"stat_t\":\"" + String(BOILER_TOPIC) + "\",\"val_tpl\":\"{{value_json." + String(OT) + String(TEMP_CUTOFF) + "}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);


    mqttclient.publish((String(HOT_WATER_HACLI_TOPIC) + "_climate/config").c_str(), ("{\"name\":\"" + String(OT) + "Hot Water\",\"uniq_id\": \"" + String(OT) + "Hot_Water\", \
\"modes\":[\"off\",\"heat\"], \
\"icon\": \"mdi:water-pump\", \
\"current_temperature_topic\":\"" + String(HOT_WATER_TOPIC) + "\", \
\"current_temperature_template\":\"{{value_json." + String(OT) + String(HOT_WATER_TEMPERATURE) + "}}\", \
\"temperature_command_topic\":\"" + String(TEMP_DHW_SET_TOPIC) + "\", \
\"temperature_state_topic\":\"" + String(HOT_WATER_TOPIC) + "\", \
\"temperature_state_template\":\"{{value_json." + String(OT) + String(HOT_WATER_TEMPERATURE_SETPOINT) + "}}\", \
\"temperature_unit\":\"C\", \
\"mode_state_topic\": \"" + String(HOT_WATER_TOPIC) + "\", \
\"mode_state_template\": \"{{value_json." + String(OT) + String(HOT_WATER_SOFTWARE_CH_STATE) + "}}\", \
\"mode_command_topic\": \"" + String(STATE_DHW_SET_TOPIC) + "\", \
\"mode_command_template\" : \"{% set values = { 'heat':'1', 'off':'0'} %}   {{ values[value] if value in values.keys() else '0' }}\", \
\"temp_step\": 0.5, \
\"precision\": 0.5, \
\"target_temp_step\": 0.5, \
\"min_temp\": " + oplo + ", \
\"max_temp\": " + ophi + ", \
\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
    mqttclient.publish((String(BOILER_HACLI_TOPIC) + "_climate/config").c_str(), ("{\"name\":\"" + String(OT) + "Boiler CO\",\"uniq_id\": \"" + String(OT) + "Boiler_CO\", \
\"modes\":[\"off\",\"heat\",\"auto\"], \
\"icon\": \"mdi:water-pump\", \
\"current_temperature_topic\":\"" + String(BOILER_TOPIC) + "\", \
\"current_temperature_template\":\"{{value_json." + String(OT) + String(BOILER_TEMPERATURE) + "}}\", \
\"temperature_command_topic\":\"" + String(TEMP_SETPOINT_SET_TOPIC) + "\", \
\"temperature_state_topic\":\"" + String(BOILER_TOPIC) + "\", \
\"temperature_state_template\":\"{{value_json." + String(OT) + String(BOILER_TEMPERATURE_SETPOINT) + "}}\", \
\"temperature_unit\":\"C\", \
\"mode_state_topic\": \"" + String(BOILER_TOPIC) + "\", \
\"mode_state_template\": \"{{value_json." + String(OT) + String(BOILER_SOFTWARE_CH_STATE_MODE) + "}}\", \
\"mode_command_topic\": \"" + String(MODE_SET_TOPIC) + "\", \
\"mode_command_template\" : \"{% set values = { 'auto':'2', 'heat':'1', 'off':'0'} %}   {{ values[value] if value in values.keys() else '0' }}\", \
\"temp_step\": 0.5, \
\"precision\": 0.5, \
\"target_temp_step\": 0.5, \
\"min_temp\": " + opcolo + ", \
\"max_temp\": " + opcohi + ", \
\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
    mqttclient.publish((String(ROOM_OTHERS_HACLI_TOPIC) + "_climate/config").c_str(), ("{\"name\":\"" + String(OT) + "Boiler RoomTemp Control CO\",\"uniq_id\": \"" + String(OT) + "Boiler_RoomTemp_Control_CO\", \
\"modes\":[\"off\",\"heat\",\"auto\"], \
\"icon\": \"mdi:water-pump\", \
\"current_temperature_topic\":\"" + String(ROOM_OTHERS_TOPIC) + "\", \
\"current_temperature_template\":\"{{value_json." + String(OT) + String(ROOM_OTHERS_TEMPERATURE) + "}}\", \
\"temperature_command_topic\":\"" + String(ROOM_TEMP_SET_TOPIC) + "\", \
\"temperature_state_topic\":\"" + String(ROOM_OTHERS_TOPIC) + "\", \
\"temperature_state_template\":\"{{value_json." + String(OT) + String(ROOM_OTHERS_TEMPERATURE_SETPOINT) + "}}\", \
\"temperature_unit\":\"C\", \
\"mode_state_topic\": \"" + String(BOILER_TOPIC) + "\", \
\"mode_state_template\": \"{{value_json." + String(OT) + String(BOILER_SOFTWARE_CH_STATE_MODE) + "}}\", \
\"mode_command_topic\": \"" + String(MODE_SET_TOPIC) + "\", \
\"mode_command_template\" : \"{% set values = { 'auto':'2', 'heat':'1', 'off':'0'} %}   {{ values[value] if value in values.keys() else '0' }}\", \
\"temp_step\": 0.5, \
\"precision\": 0.5, \
\"target_temp_step\": 0.5, \
\"min_temp\": " + roomtemplo + ", \
\"max_temp\": " + roomtemphi + ", \
\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
    mqttclient.publish((String(ROOM_OTHERS_HACLI_TOPIC) + "cutoff_climate/config").c_str(), ("{\"name\":\"" + String(OT) + "CutoffTemp\",\"uniq_id\": \"" + String(OT) + "CutoffTemp\", \
\"modes\":\"\", \
\"icon\": \"mdi:water-pump\", \
\"current_temperature_topic\":\"" + String(NEWS_GET_TOPIC) + "\", \
\"current_temperature_template\":\"{{value}}\", \
\"temperature_command_topic\":\"" + String(TEMP_CUTOFF_SET_TOPIC) + "\", \
\"temperature_state_topic\":\"" + String(BOILER_TOPIC) + "\", \
\"temperature_state_template\":\"{{value_json." + String(OT) + String(TEMP_CUTOFF) + "}}\", \
\"temperature_unit\":\"C\", \
\"temp_step\": 0.5, \
\"precision\": 0.5, \
\"target_temp_step\": 0.5, \
\"min_temp\": " + cutofflo + ", \
\"max_temp\": " + cutoffhi + ", \
\"qos\":" + String(QOS) + "," + String(deviceid) + "}").c_str(), mqtt_Retain);
  }


}

void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
#ifdef enableMQTT
  const String topicStr(topic);
  String payloadStr = convertPayloadToStr(payload, length);
  payloadStr.trim();
//  sprintf(log_chars, "MQTT callback Topic: %s", String(topicStr).c_str());
//  log_message(log_chars);
  sprintf(log_chars, "MQTT callback Topic: %s, Received message ...: ", String(topicStr).c_str());
  log_message(log_chars);
  #ifdef debug0xtu //na seriaprint wywale exc28
  sprintf(log_chars, "MQTT callback Topic: %s, Received message ...: %s", String(topicStr).c_str(), String(payloadStr).c_str());
  log_message(log_chars);

    Serial.println(TEMP_SETPOINT_SET_TOPIC);
    Serial.println(MODE_SET_TOPIC);
    Serial.println(TEMP_DHW_SET_TOPIC);
    Serial.println(STATE_DHW_SET_TOPIC);
    Serial.println(SETPOINT_OVERRIDE_SET_TOPIC);
    Serial.println(SETPOINT_OVERRIDE_RESET_TOPIC);

    Serial.println(NEWS_GET_TOPIC);
    Serial.println(COPUMP_GET_TOPIC);
    Serial.println(TEMP_CUTOFF_SET_TOPIC);
    Serial.println(ROOMS_F1_GET_TOPIC);
    Serial.println(ROOMS_F2_GET_TOPIC);
    Serial.println(ROOM_TEMP_SET_TOPIC);

  #endif

//It cannon be else if because one topic can contain more values ;()
  if (topicStr.indexOf(String(ROOMS_F1_GET_TOPIC))>=0 and payloadStr.indexOf(String(roomF1temp_json))>=0) //topic min temp and max setpoint from floor 1
  {
    String ident = "Floor1temp ";
    //   WebSerial.println("Payload: " + String(payloadStr));
    if (PayloadtoValidFloatCheck(getJsonVal(payloadStr, String(roomF1temp_json)))) //wrong value are displayed in function
    {
      floor1_temp = PayloadtoValidFloat(getJsonVal(payloadStr, String(roomF1temp_json)), true, roomtemplo, roomtemphi);
      sprintf(log_chars, "%s set to: %s", ident.c_str(), String(floor1_temp).c_str());
      log_message(log_chars);
      receivedmqttdata = true;
    } else {
      sprintf(log_chars, "%s is not a valid number, ignoring..., payloadStr: %s, (%s)", ident.c_str(), payloadStr.c_str(), String(payloadStr.length()).c_str());
      log_message(log_chars);
    }
    if (PayloadtoValidFloatCheck(getJsonVal(payloadStr, String(roomF1tempset_json)))) //wrong value are displayed in function
    {
      floor1_tempset = PayloadtoValidFloat(getJsonVal(payloadStr, String(roomF1tempset_json)), true, roomtemplo, roomtemphi);
      sprintf(log_chars, "%s Setpoint set to: %s", ident.c_str(), String(floor1_tempset).c_str());
      log_message(log_chars);
      // receivedmqttdata = true;z
    } else {
      sprintf(log_chars, "%s Setpoint is not a valid number, ignoring..., payloadStr: %s (%s)", ident.c_str(), payloadStr.c_str(), String(payloadStr.length()).c_str());
      log_message(log_chars);
    }
  }
   if (topicStr.indexOf(String(ROOMS_F2_GET_TOPIC))>=0 and payloadStr.indexOf(String(roomF2temp_json))>=0) //topic min temp and max setpoint from floor 1
  {
    String ident = "Floor2temp ";
    //   WebSerial.println("Payload: " + String(payloadStr));
    if (PayloadtoValidFloatCheck(getJsonVal(payloadStr, String(roomF2temp_json)))) //wrong value are displayed in function
    {
      floor2_temp = PayloadtoValidFloat(getJsonVal(payloadStr, roomF2temp_json), true, roomtemplo, roomtemphi);
      sprintf(log_chars, "%s set to: %s", ident.c_str(), String(floor2_temp).c_str());
      log_message(log_chars);
      receivedmqttdata = true;
    } else {
      sprintf(log_chars, "%s Setpoint is not a valid number, ignoring..., payloadStr: %s (%s)", ident.c_str(), payloadStr.c_str(), String(payloadStr.length()).c_str());
      log_message(log_chars);
    }
    if (PayloadtoValidFloatCheck(getJsonVal(payloadStr, String(roomF2tempset_json)))) //wrong value are displayed in function
    {
      floor2_tempset = PayloadtoValidFloat(getJsonVal(payloadStr, roomF2tempset_json), true, roomtemplo, roomtemphi);
      sprintf(log_chars, "%s Setpoint set to: %s", ident.c_str(), String(floor2_tempset).c_str());
      log_message(log_chars);
      receivedmqttdata = true;
    } else {
      sprintf(log_chars, "%s Setpoint is not a valid number, ignoring..., payloadStr: %s (%s)", ident.c_str(), payloadStr.c_str(), String(payloadStr.length()).c_str());
      log_message(log_chars);
    }
  }
  if (topicStr.indexOf(String(NEWS_GET_TOPIC))>=0 and payloadStr.indexOf(String(NEWStemp_json))>=0)              //NEWS averange temp -outside temp
  {
    String ident = "NEWS temp ";
    if (PayloadtoValidFloatCheck(getJsonVal(payloadStr, String(NEWStemp_json))))          //invalid val is displayed in funct
    {
      temp_NEWS = PayloadtoValidFloat(getJsonVal(payloadStr, String(NEWStemp_json)), true);   //true to get output to serial and webserial
      sprintf(log_chars, "%s updated from MQTT to: %s", ident.c_str(), String(temp_NEWS).c_str());
      log_message(log_chars);
      lastNEWSSet = millis();
      temp_NEWS_count = 0;
      //      receivedmqttdata = true;    //makes every second run mqtt send and influx
    } else {
      sprintf(log_chars, "%s not updated from MQTT: %s, %s", ident.c_str(), String(getJsonVal(payloadStr, String(NEWStemp_json))).c_str(), String(PayloadtoValidFloatCheck(getJsonVal(payloadStr, String(NEWStemp_json)))).c_str());
      log_message(log_chars);
    }
  }
  if (topicStr.indexOf(String(ROOM_TEMP_SET_TOPIC))>=0 and !payloadStr.startsWith("{"))           // Rooms autosetp.roomtemp for auto mode
  {
    String ident = "Rooms Current roomtemp ";
    if (PayloadtoValidFloatCheck(payloadStr))  //wrong value are displayed in function
    {
      roomtempSet = PayloadtoValidFloat(payloadStr, true, roomtemplo, roomtemphi);
      lastroomtempSet = 0;
      sprintf(log_chars, "%s set to: %s", ident.c_str(), String(roomtempSet).c_str());
      log_message(log_chars);
      receivedmqttdata = true;
    } else {
      sprintf(log_chars, "%s is not a valid number, ignoring..., payloadStr: %s", ident.c_str(), payloadStr.c_str());
      log_message(log_chars);
    }
  }
  if (topicStr.indexOf(String(TEMP_SETPOINT_SET_TOPIC))>=0  and !payloadStr.startsWith("{"))      //Room Target roomtempSet for automode
  {
    String ident = "Room Target roomtempSet ";
    if (PayloadtoValidFloatCheck(payloadStr))  //wrong value are displayed in function
    {
      tempBoilerSet = PayloadtoValidFloat(payloadStr, true, roomtemplo, roomtemphi);
      //      op_override = tempBoilerSet; // when no auto heating then this is temp to heat CO
      sprintf(log_chars, "%s set to: %s", ident.c_str(), String(tempBoilerSet).c_str());
      log_message(log_chars);
      receivedmqttdata = true;
    } else {
      sprintf(log_chars, "%s is not a valid number, ignoring..., payloadStr: %s", ident.c_str(), payloadStr.c_str());
      log_message(log_chars);
    }
  }
  if (topicStr.indexOf(String(MODE_SET_TOPIC))>=0  and !payloadStr.startsWith("{"))              //mode set topic
  {
    String ident = "Mode Set ";
    payloadStr.toUpperCase();
    if (PayloadStatus(payloadStr, true))
    {
      heatingEnabled = true;
      receivedmqttdata = true;
      sprintf(log_chars, "%s to: CO mode  %s", ident.c_str(), String(payloadStr).c_str());
      log_message(log_chars);
    }
    else if (PayloadStatus(payloadStr, false))
    {
      heatingEnabled = false;
      receivedmqttdata = true;
      sprintf(log_chars, "%s to: CO mode  %s", ident.c_str(), String(payloadStr).c_str());
      log_message(log_chars);
    }
    else if (payloadStr == "AUTO" or payloadStr == "2")
    {
      heatingEnabled = true;
      if (heatingEnabled) automodeCO = true;
      receivedmqttdata = true;
      sprintf(log_chars, "%s to: CO mode  %s", ident.c_str(), String(payloadStr).c_str());
      log_message(log_chars);
    } else {
      sprintf(log_chars, "%s to: Unknown mode  %s", ident.c_str(), String(payloadStr).c_str());
      log_message(log_chars);
    }
  }
  if (topicStr.indexOf(String(TEMP_DHW_SET_TOPIC))>=0  and !payloadStr.startsWith("{"))    // dhwTarget
  {
    String ident = "DHW target ";
    if (PayloadtoValidFloatCheck(payloadStr))  //wrong value are displayed in function
    {
      dhwTarget = PayloadtoValidFloat(payloadStr, true, oplo, ophi);
      sprintf(log_chars, "%s to: %s", ident.c_str(), String(dhwTarget).c_str());
      log_message(log_chars);
      receivedmqttdata = true;
    } else {
      sprintf(log_chars, "%s is not a valid number, ignoring..., payloadStr: %s", ident.c_str(), payloadStr.c_str());
      log_message(log_chars);
    }
  }
  if (topicStr.indexOf(String(STATE_DHW_SET_TOPIC))>=0 and !payloadStr.startsWith("{"))   // enableHotWater
  {
    String ident = "DHW State enableHotWater ";
    receivedmqttdata = true;
    if (PayloadStatus(payloadStr, true)) {
      enableHotWater = true;
    }
    else if (PayloadStatus(payloadStr, false)) {
      enableHotWater = false;
    }
    else
    {
      receivedmqttdata = false;
      sprintf(log_chars, "%s to: Unknown %s", ident.c_str(), String(payloadStr).c_str());
      log_message(log_chars);
    }
    if (receivedmqttdata) {
      sprintf(log_chars, "%s to:  %s", ident.c_str(), String(enableHotWater ? "heat" : "off").c_str());
      log_message(log_chars);
    }
  }
  if (topicStr.indexOf(String(TEMP_CUTOFF_SET_TOPIC))>=0 and !payloadStr.startsWith("{"))         //cutOffTemp
  {
    String ident = "cutOffTemp ";
    if (PayloadtoValidFloatCheck(payloadStr))  //wrong value are displayed in function
    {
      cutOffTemp = PayloadtoValidFloat(payloadStr, true, cutofflo, cutoffhi);
      sprintf(log_chars, "%s to: %s", ident.c_str(), String(cutOffTemp).c_str());
      log_message(log_chars);
      lastcutOffTempSet = millis();
      receivedmqttdata = true;
    } else {
      sprintf(log_chars, "%s is not a valid number, ignoring..., payloadStr: %s", ident.c_str(), payloadStr.c_str());
      log_message(log_chars);
    }
  }

    //CO Pump Status #define COPumpStatus_json "CO0_boilerroom_pump2CO"
    //#define WaterPumpStatus_json "CO0_boilerroom_pump1Water"
    if (topicStr.indexOf(String(COPUMP_GET_TOPIC))>=0 and payloadStr.indexOf(String(COPumpStatus_json))>=0)                                                                   //external CO Pump Status
    {
      String ident = "Wood/coax CO Pump Status ";
      receivedmqttdata = true;
      if (PayloadStatus(getJsonVal(payloadStr, String(COPumpStatus_json)), true)) {
        CO_PumpWorking = true;
      }
      else if (PayloadStatus(getJsonVal(payloadStr, String(COPumpStatus_json)), false)) {
        CO_PumpWorking = false;
      }
      else
      {
        receivedmqttdata = false;
        sprintf(log_chars, "%s to: Unknown %s", ident.c_str(), String(payloadStr).c_str());
        log_message(log_chars);
      }
      if (receivedmqttdata) {
        sprintf(log_chars, "%s to:  %s", ident.c_str(), String(CO_PumpWorking ? "Active" : "Disabled").c_str());
        log_message(log_chars);
      }
    }
      //CO Pump Status #define COPumpStatus_json "CO0_boilerroom_pump2CO"
      //#define WaterPumpStatus_json "CO0_boilerroom_pump1Water"
      if (topicStr.indexOf(String(COPUMP_GET_TOPIC))>=0 and payloadStr.indexOf(String(WaterPumpStatus_json))>=0)                                                                   //external CO Pump Status
      {
        String ident = "Wood/coax Water Pump Status ";
        receivedmqttdata = true;
        if (PayloadStatus(getJsonVal(payloadStr, String(WaterPumpStatus_json)), true)) {
          Water_PumpWorking = true;
        }
        else if (PayloadStatus(getJsonVal(payloadStr, String(WaterPumpStatus_json)), false)) {
          Water_PumpWorking = false;
        }
        else
        {
          receivedmqttdata = false;
          sprintf(log_chars, "%s to: Unknown %s", ident.c_str(), String(payloadStr).c_str());
          log_message(log_chars);
        }
        if (receivedmqttdata) {
          sprintf(log_chars, "%s to:  %s", ident.c_str(), String(Water_PumpWorking ? "Active" : "Disabled").c_str());
          log_message(log_chars);
        }
      }

#endif
}

void mqttReconnect_subscribe_list()
{
  log_message((char*)F("MQTT Reconnect Subscribe List..."));
  mqttclient.subscribe(String(TEMP_SETPOINT_SET_TOPIC).c_str());
  mqttclient.subscribe(String(MODE_SET_TOPIC).c_str());
//  mqttclient.subscribe(String(ROOM_TEMP_SET_TOPIC).c_str());
  mqttclient.subscribe(String(TEMP_DHW_SET_TOPIC).c_str());
  mqttclient.subscribe(String(STATE_DHW_SET_TOPIC).c_str());
  mqttclient.subscribe(String(SETPOINT_OVERRIDE_SET_TOPIC).c_str());
  mqttclient.subscribe(String(SETPOINT_OVERRIDE_RESET_TOPIC).c_str());
  mqttclient.subscribe(String(NEWS_GET_TOPIC).c_str());
  mqttclient.subscribe(String(COPUMP_GET_TOPIC).c_str());
  mqttclient.subscribe(String(TEMP_CUTOFF_SET_TOPIC).c_str());
  mqttclient.subscribe(String(ROOMS_F1_GET_TOPIC).c_str());
  mqttclient.subscribe(String(ROOMS_F2_GET_TOPIC).c_str());


}
