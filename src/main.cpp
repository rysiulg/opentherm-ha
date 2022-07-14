/*************************************************************
  This example runs directly on ESP8266 chip.

  Please be sure to select the right ESP8266 module
  in the Tools -> Board -> WeMos D1 Mini

  Adjust settings in Config.h before run
 *************************************************************/

//#include <Arduino.h>


// ESP8266WebServer server(80);
#include "main.h"
#include "common_functions.h"
#include "websrv_ota.h"

void recvMsg(uint8_t *data, size_t len)
{ // for WebSerial
#ifdef enableWebSerial
  log_message((char*)F("Received Data..."));
  String d = "";
  for (size_t i = 0; i < len; i++)
  {
    d += char(data[i]);
  }
  d.toUpperCase();
  sprintf(log_chars, "Received: %s", String(d).c_str());
  log_message(log_chars);

  if (d == "RESTART")
  {
    log_message((char*)F("OK. Restarting... by command..."));
    restart();
  }
  if (d == "RECONNECT")
  {
    mqtt_reconnect();
  }
  if (d == "ROOMTEMP+")
  {
    float startroomtemp = roomtemp;
    if (roomtemp < roomtemphi) {roomtemp = roomtemp + 0.5;}
    tmanual = true;
    lastTempSet = millis();
    sprintf(log_chars, "Change ROOMTEMP+ from: %s to: %s", String(startroomtemp).c_str(), String(roomtemp).c_str());
    log_message(log_chars);
  }
  if (d == "ROOMTEMP-")
  {
    float startroomtemp = roomtemp;
    if (roomtemp > roomtemplo) {roomtemp = roomtemp - 0.5;}
    lastTempSet = millis();
    tmanual = true;
    sprintf(log_chars, "Change ROOMTEMP- from: %s to: %s", String(startroomtemp).c_str(), String(roomtemp).c_str());
    log_message(log_chars);
  }
  if (d == "ROOMTEMP0")
  {
    tmanual = !tmanual;
    lastTempSet = millis();
    sprintf(log_chars, "Toggle ROOMTEMP0 from: %s to: %s", String(!tmanual?"MANUAL":"AUTO").c_str(), String(tmanual?"MANUAL":"AUTO").c_str());
    log_message(log_chars);
  }
  if (d == "SAVE")
  {
    sprintf(log_chars,"Saving config to EEPROM memory by command...  CONFIG Size: %s",String(sizeof(CONFIGURATION)).c_str());
    log_message(log_chars,0);
    SaveConfig();
  }
  if (d == "RESET_CONFIG")
  {
    sprintf(log_chars,"RESET config to DEFAULT VALUES and restart...  CONFIG Size: %s",String(sizeof(CONFIGURATION)).c_str());
    log_message(log_chars,0);
    CONFIGURATION.version[0] = 'R';
    CONFIGURATION.version[1] = 'E';
    CONFIGURATION.version[2] = 'S';
    CONFIGURATION.version[3] = 'E';
    CONFIGURATION.version[4] = 'T';
    SaveConfig();
    restart();
  }
  if (d == "RESET_FLAMETOTAL")
  {
    log_message((char*)F("RESET flame Total var to 0..."),0);
    flame_used_power_kwh=0;
    SaveConfig();
  }

  if (d == "CO")
  {
    // espClient.connect("esp-b2c08e/dallThermometerS");
    mqttclient.disconnect();

    espClient.stop();
    String host = "esp-b2c08e";
    if (espClient.connect("esp-b2c08e", 80))
    {
      WebSerial.println("connected]");

      WebSerial.println("[Sending a request]");
      String url = "/dallThermometerS";
      espClient.print(String("GET /") + url + " HTTP/1.1\r\n" +
                      "Host: " + host + "\r\n" +
                      "Connection: close\r\n" +
                      "\r\n");

      WebSerial.println("[Response:]");
      while (espClient.connected())
      {
        String line = espClient.readStringUntil('\r');
        WebSerial.print(line);
        WebSerial.println("Liczba: ");
        WebSerial.println(isValidNumber(line) ? String(line.toFloat()) : "NaN");
      }
      espClient.stop();
      WebSerial.println("\n[Disconnected]");
    }
    else
    {
      WebSerial.println("connection failed!]");
      espClient.stop();
    }
  }
  if (d == "HELP")
  {
    log_message((char*)F("KOMENDY:\n \
      ROOMTEMP0        -Przelacza temperature z pokoju na automat,\n \
      ROOMTEMP+        -Zwiększa wartość temperatury z pokoju o 0,5 stopnia,\n \
      ROOMTEMP-        -Zmniejsza wartość temperatury z pokoju o 0,5 stopnia,\n \
      CO               -Testowo -rozlacza Client i prubuje pobrac z www?,\n \
      RESTART          -Uruchamia ponownie układ,\n \
      RECONNECT        -Dokonuje ponownej próby połączenia z bazami,\n \
      SAVE             -Wymusza zapis konfiguracji,\n \
      RESET_CONFIG     -UWAGA!!!! Resetuje konfigurację do wartości domyślnych\n \
      RESET_FLAMETOTAL -UWAGA!!!! Resetuje licznik płomienia-zużycia kWh na 0"),0);
  }
  #endif
}

void updateDatatoWWW()
{
  //String dana = {"DHWTemp",DHW_Temp}
  String ptr = "\0";
  u_int i = 0;
  AllSensorsStruct[i].placeholder = "uptimedana";
  AllSensorsStruct[i].Value = String(uptimedana(0));
  i++;
  AllSensorsStruct[i].placeholder = "temp_NEWS";
  AllSensorsStruct[i].Value = String(temp_NEWS);
  i++;
  AllSensorsStruct[i].placeholder = "tempBoiler";
  AllSensorsStruct[i].Value = String(tempBoiler,1);
  i++; //tempBoilerSet
  AllSensorsStruct[i].placeholder = "sliderValue1";
  if (receivedwebsocketdata or receivedmqttdata) {
  tempBoilerSet = PayloadtoValidFloat(AllSensorsStruct[i].Value, true, oplo, ophi);
  op_override = tempBoilerSet; // when no auto heating then this is temp to heat CO
  sprintf(log_chars,"Allsensors for %s, val: %s konw.: %s","tempBoilerSet", AllSensorsStruct[i].Value.c_str(), String(tempBoilerSet).c_str());
  log_message(log_chars);
  }
  AllSensorsStruct[i].Value = String(tempBoilerSet,1);
  i++;
  AllSensorsStruct[i].placeholder = "retTemp";
  AllSensorsStruct[i].Value = String(retTemp,1);
  i++;
  AllSensorsStruct[i].placeholder = "tempCWU";
  AllSensorsStruct[i].Value = String(tempCWU,1);
  i++;      //dhwTarget
  AllSensorsStruct[i].placeholder = "sliderValue2";
 if (receivedwebsocketdata or receivedmqttdata) {
  dhwTarget = PayloadtoValidFloat(AllSensorsStruct[i].Value, true, oplo, ophi);
  sprintf(log_chars,"Allsensors for %s, val: %s konw.: %s","dhwTarget", AllSensorsStruct[i].Value.c_str(), String(dhwTarget).c_str());
  log_message(log_chars);
 }
  AllSensorsStruct[i].Value = String(dhwTarget,1);
  i++;      //cutOffTemp
  AllSensorsStruct[i].placeholder = "sliderValue3";
  if (receivedwebsocketdata or receivedmqttdata) {
    cutOffTemp = PayloadtoValidFloat(AllSensorsStruct[i].Value, true, cutofflo, cutoffhi);
    lastcutOffTempSet = millis();
    sprintf(log_chars,"Allsensors for %s, val: %s konw.: %s","cutOffTemp", AllSensorsStruct[i].Value.c_str(), String(cutOffTemp).c_str());
    log_message(log_chars);
  }
  AllSensorsStruct[i].Value = String(cutOffTemp,1);
  i++;
  AllSensorsStruct[i].placeholder = "roomtemp";
  AllSensorsStruct[i].Value = String(roomtemp,1);
  i++;      //sp
  AllSensorsStruct[i].placeholder = "sliderValue4";  //Room Target sp
  if (receivedwebsocketdata or receivedmqttdata) {
    sp = PayloadtoValidFloat(AllSensorsStruct[i].Value, true, roomtemplo, roomtemphi);
    sprintf(log_chars,"Allsensors for %s, val: %s konw.: %s","sp", AllSensorsStruct[i].Value.c_str(), String(sp).c_str());
    log_message(log_chars);
  }
  AllSensorsStruct[i].Value = String(sp,1);
  i++;
  AllSensorsStruct[i].placeholder = "lastNEWSSet";
  AllSensorsStruct[i].Value = String(uptimedana(lastNEWSSet));
  i++;
  AllSensorsStruct[i].placeholder = "boilermodewww";
  if (receivedwebsocketdata or receivedmqttdata) {
    if (PayloadStatus(AllSensorsStruct[i].Value, true))
            {
              heatingEnabled = true;
              automodeCO = false;
              tempBoilerSet = op_override;
              sprintf(log_chars,"CO mode: %s", AllSensorsStruct[i].Value.c_str());
              log_message(log_chars);
            }
            else if (PayloadStatus(AllSensorsStruct[i].Value, false))
            {
              heatingEnabled = false;
              automodeCO = false;
              sprintf(log_chars,"CO mode: %s", AllSensorsStruct[i].Value.c_str());
              log_message(log_chars);
            }
            else if (AllSensorsStruct[i].Value == "AUTO" or AllSensorsStruct[i].Value == "2")
            {
              automodeCO = true;
              receivedmqttdata = true;
              sprintf(log_chars,"CO mode: %s", AllSensorsStruct[i].Value.c_str());
              log_message(log_chars);
            } else {
              sprintf(log_chars,"Unknown mode: %s", AllSensorsStruct[i].Value.c_str());
              log_message(log_chars);
            }
  }
  AllSensorsStruct[i].Value = String(automodeCO?"ON":"OFF");
  i++;
  AllSensorsStruct[i].placeholder = "boilerhwwww";
    if (receivedwebsocketdata or receivedmqttdata) {
      if (PayloadStatus(AllSensorsStruct[i].Value, true)) {
        enableHotWater = true;
      } else
      if (PayloadStatus(AllSensorsStruct[i].Value, false)) {
        enableHotWater = false;
      } else
      {
        sprintf(log_chars,"Unknown mode: %s", AllSensorsStruct[i].Value.c_str());
        log_message(log_chars);
      }
      sprintf(log_chars,"DHW State enableHotWater: %s", enableHotWater ? "heat" : "off");
      log_message(log_chars);
    }
  AllSensorsStruct[i].Value = String(enableHotWater ? "heat" : "off");
  i++;
  AllSensorsStruct[i].placeholder = "naglowekdane";
  AllSensorsStruct[i].Value = String("naglowekdane");

      ptr = "\0";
      if (status_FlameOn) {
        ptr += "<i class='fas fa-fire' style='color: red'></i>"; ptr += "<span class='dht-labels'>"+String(Flame_Active_Flame_level)+"</span><B>"+ String(flame_level,0)+"<sup class=\"units\">&#37;</sup></B>";
        ptr += "<br>";
      }
      if (status_Fault) ptr += "<span class='dht-labels'><B>!!!!!!!!!!!!!!!!! status_Fault !!!!!!!<br></B></span>";
      if (heatingEnabled) ptr += "<span class='dht-labels'><B>"+String(BOILER_HEAT_ON)+"<br></B></span>";
      if (status_CHActive) ptr += "<font color=\"red\"><span class='dht-labels'><B>"+String(BOILER_IS_HEATING)+"<br></B></span></font>";
      if (enableHotWater) ptr += "<span class='dht-labels'><B>"+String(DHW_HEAT_ON)+"<br></B></span>";
      if (status_WaterActive) ptr += "<font color=\"red\"><span class='dht-labels'><B>"+String(Boiler_Active_heat_DHW)+"<br></B></span></font>";
      if (status_Cooling) ptr += "<font color=\"orange\"><span class='dht-labels'><B>"+String(CoolingMode)+"<br></B></span></font>";
      if (status_Diagnostic) ptr += "<font color=\"darkred\"><span class='dht-labels'><B>"+String(DiagMode)+"<br></B></span></font>";
      if (CO_PumpWorking) ptr += "<font color=\"blue\"><span class='dht-labels'><B>"+String(Second_Engine_Heating_PompActive_Disable_heat)+"<br></B><br></span></font>";
      if (Water_PumpWorking) ptr += "<font color=\"blue\"><span class='dht-labels'><B>"+String(Second_Engine_Heating_Water_PompActive)+"<br></B><br></span></font>";
      if (flame_time>0) ptr+= "<font color=\"green\"><span class='dht-labels'>"+String(Flame_time)+"<B>"+uptimedana(millis()-flame_time)+"<br></B><br></span></font>";
  i++;
  AllSensorsStruct[i].placeholder = "Statusy";
  AllSensorsStruct[i].Value = String(ptr);
  i++;
  ptr = "\0";
  ptr += String(Flame_total)+"<B>"+String(flame_used_power_kwh,4)+"kWh</B>";
  AllSensorsStruct[i].placeholder = "UsedMedia";
  AllSensorsStruct[i].Value = String(ptr);
receivedwebsocketdata = false;
  notifyClients(getValuesToWebSocket_andWebProcessor(ValuesToWSWPinJSON));  //moze nie potrzebne
}




void IRAM_ATTR handleInterrupt()
{
  ot.handleInterrupt();
}

float getTemp()
{
    if (floor2_tempset!=InitTemp && floor1_tempset!=InitTemp) sp=(floor2_tempset+floor1_tempset)/2; //{roomtemp_last=roomtemp; roomtemp=(floor2_tempset+floor1_tempset)/2;}
    if (floor2_tempset!=InitTemp && floor1_tempset==InitTemp) {sp=floor2_tempset;}
    if (floor2_tempset==InitTemp && floor1_tempset!=InitTemp) {sp=floor1_tempset;}
    if (floor2_temp!=InitTemp && floor1_temp!=InitTemp) {roomtemp=(floor2_temp+floor1_temp)/2;}
    if (floor2_temp!=InitTemp && floor1_temp==InitTemp) {roomtemp=floor2_temp;}
    if (floor2_temp==InitTemp && floor1_temp!=InitTemp) {roomtemp=floor1_temp;}

  // unsigned long now = millis();
  // if ((now - lastTempSet) > extTempTimeout_ms)
  // {
  //   lastTempSet = now;
  //   WebSerial.print(F("Update dallas temp."));
  //   return sensors.getTempCByIndex(0);
  // }
  // else
    return roomtemp;
}

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
  sprintf(log_chars,"sp=%s, pv=%s, dt=%s, op=%s, P=%s, I=%s, tNEWS=%s", String(sp).c_str(), String(pv).c_str(), String(dt).c_str(), String(op).c_str(), String(P).c_str(), String(I).c_str(), String(temp_NEWS).c_str());
  log_message(log_chars);
  return op;
}

// This function calculates temperature every second.
void opentherm_update_data(unsigned long mqttdallas)
{
  // Set/Get Boiler Status
  bool enableCooling = false;
  receivedmqttdata = false;

  if (temp_NEWS < cutOffTemp)
    heatingEnabled = true;
  if (CO_PumpWorking)
    heatingEnabled = false;
  if (temp_NEWS > (cutOffTemp + 0.9))
    heatingEnabled = false;

  //if (tmanual == false)
  roomtemp =  getTemp();
  if (roomtemp == -127)
    roomtemp = sp;
  if (roomtemp_last == -127)
    roomtemp_last = sp;

  unsigned long response = ot.setBoilerStatus(heatingEnabled, enableHotWater, enableCooling); // enableOutsideTemperatureCompensation
  OpenThermResponseStatus responseStatus = ot.getLastResponseStatus();
  if (responseStatus != OpenThermResponseStatus::SUCCESS)
  {

    LastboilerResponseError = String(response, HEX);
    sprintf(log_chars,"!!!!!!!!!!!Error: Invalid boiler response %s", LastboilerResponseError.c_str());
    log_message(log_chars);
  } else
  {
    ot.setDHWSetpoint(dhwTarget);
    float op = 0;
    unsigned long now = millis();
    new_ts = millis();
    dt = (new_ts - ts) / 1000.0;
    ts = new_ts;
    op = pid(sp, roomtemp, roomtemp_last, ierr, dt);
    if ((now - lastSpSet <= spOverrideTimeout_ms))
    {
      op = noCommandSpOverride;
    }
    if (automodeCO)
    {
      // WebSerial.println(F("tryb AutomodeCO"));
      tempBoilerSet = op;
    }
    else
    {
      tempBoilerSet = op_override;
    }
    ot.setBoilerTemperature(tempBoilerSet);

    roomtemp_last = roomtemp;
    status_CHActive = ot.isCentralHeatingActive(response);
    status_WaterActive = ot.isHotWaterActive(response);
    bool status_flame_tmp = status_FlameOn;
    status_FlameOn = ot.isFlameOn(response);
    if (status_flame_tmp != status_FlameOn) {
      if (status_FlameOn) {start_flame_time=millis();} else start_flame_time=0;  //After change flame status If flame is on get timer, else reset timer
      flame_time=start_flame_time;
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

String Boiler_Mode()
{
  if (automodeCO)
    return "auto";
  else
  {
    if (heatingEnabled)
      return "heat";
    else
      return "off";
  }
}

void updateInfluxDB()
{
#ifdef ENABLE_INFLUX
  String boilermode = Boiler_Mode();
  InfluxSensor.clearFields();
  // Report RSSI of currently connected network
  InfluxSensor.addField("rssi_BCO", (WiFi.RSSI()));
  InfluxSensor.addField("CRT_BCO", (runNumber));
  InfluxSensor.addField(String(ROOM_OTHERS_TEMPERATURE), roomtemp);
  InfluxSensor.addField(String(ROOM_OTHERS_TEMPERATURE_SETPOINT), sp);
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
  InfluxSensor.addField(String(TEMP_CUTOFF), cutOffTemp);
  InfluxSensor.addField(String(DIAGS_OTHERS_FAULT), status_Fault ? "1" : "0");
  InfluxSensor.addField(String(DIAGS_OTHERS_DIAG), status_Diagnostic ? "1" : "0");
  InfluxSensor.addField(String(INTEGRAL_ERROR_GET_TOPIC), ierr);
  InfluxSensor.addField(String(LOG_GET_TOPIC), LastboilerResponseError);

  // Print what are we exactly writing
    sprintf(log_chars,"Writing to InfluxDB:  %s", String(InfluxClient.getLastErrorMessage()).c_str());
    log_message(log_chars);
  //WebSerial.println(InfluxClient.pointToLineProtocol(InfluxSensor));
  // Write point
  if (!InfluxClient.writePoint(InfluxSensor))
  {
    sprintf(log_chars,"InfluxDB write failed: %s", String(InfluxClient.getLastErrorMessage()).c_str());
    log_message(log_chars);
  }
#endif
}

// This function  sends data to MQTT .
void updateMQTTData()
{
  const String payloadvalue_startend_val = ""; // value added before and after value send to mqtt queue
  const String payloadON = "1";
  const String payloadOFF = "0";

  mqttclient.publish(LOG_GET_TOPIC.c_str(), LastboilerResponseError.c_str());

  String boilermode = Boiler_Mode();

    //unsigned long flame_elapsed_time = (millis()-flame_time);
    //String flame_used_energy=String(((flame_used_power))/1,4);  //unit kWh
    // WebSerial.print(String(millis())+": flame_used_power kWh: "); WebSerial.println(String(flame_used_power_kwh));
    // WebSerial.print(String(millis())+": flame_elapsed_time: "); WebSerial.println(String(flame_elapsed_time));
    // WebSerial.print(String(millis())+": flame_W used: "); WebSerial.println(String(flame_used_energy));
    // WebSerial.println("Flame level: "+String(flame_level));
    flame_used_power=0;
    start_flame_time=0;
    //flame_time=0;


  if (status_Fault)
   { sprintf(log_chars,"Błąd: %s", String(status_Fault ? "on" : "off").c_str());
    log_message(log_chars);}
  if (status_CHActive)
    {sprintf(log_chars,"Status_CHActive: %s", String(status_CHActive ? "on" : "off").c_str());
    log_message(log_chars);}
  if (status_WaterActive)
    {sprintf(log_chars,"Status_WaterActive: %s", String(status_WaterActive ? "on" : "off").c_str());
    log_message(log_chars);}
  if (enableHotWater)
    {sprintf(log_chars,"EnableHW: %s", String(enableHotWater ? "on" : "off").c_str());
    log_message(log_chars);}
  if (status_FlameOn)
    {sprintf(log_chars,"Status_FlameOn: %s", String(status_FlameOn ? "on" : "off").c_str());
    log_message(log_chars);}
  if (status_Cooling)
    {sprintf(log_chars,"Status_Cooling: %s", String(status_Cooling ? "on" : "off").c_str());
    log_message(log_chars);}
  if (status_Diagnostic)
    {sprintf(log_chars,"Status_Diagnostic: %s", String(status_Diagnostic ? "on" : "off").c_str());
    log_message(log_chars);}

  mqttclient.publish(ROOM_OTHERS_TOPIC.c_str(),
                 ("{\"rssi\":"+ String(WiFi.RSSI()) + \
                  ",\"CRT\":"+ String(runNumber) + \
                  ",\"" + OT + ROOM_OTHERS_TEMPERATURE + "\": " + payloadvalue_startend_val + String(roomtemp) + payloadvalue_startend_val +
                  ",\"" + OT + ROOM_OTHERS_TEMPERATURE_SETPOINT + "\": " + payloadvalue_startend_val + String(sp) + payloadvalue_startend_val +
                  ",\"" + OT + ROOM_OTHERS_PRESSURE + "\": " + payloadvalue_startend_val + String(pressure) + payloadvalue_startend_val +
                  "}").c_str(), mqtt_Retain); //"heat" : "off")

  mqttclient.publish(HOT_WATER_TOPIC.c_str(),
                 ("{\"" + OT + HOT_WATER_TEMPERATURE + "\": " + payloadvalue_startend_val + String(tempCWU) + payloadvalue_startend_val +
                  ",\"" + OT + HOT_WATER_TEMPERATURE_SETPOINT + "\": " + payloadvalue_startend_val + String(dhwTarget, 1) + payloadvalue_startend_val +
                  ",\"" + OT + HOT_WATER_CH_STATE + "\": " + payloadvalue_startend_val + String(status_WaterActive ? payloadON : payloadOFF) + payloadvalue_startend_val +
                  ",\"" + OT + HOT_WATER_SOFTWARE_CH_STATE + "\": \"" + String(enableHotWater ? "heat" : "off") + "\"" +
                  "}").c_str(), mqtt_Retain); //"heat" : "off")

  mqttclient.publish(BOILER_TOPIC.c_str(),
                 ("{\"" + OT + BOILER_TEMPERATURE + "\": " + payloadvalue_startend_val + String(tempBoiler) + payloadvalue_startend_val +
                  ",\"" + OT + BOILER_TEMPERATURE_RET + "\": " + payloadvalue_startend_val + String(retTemp) + payloadvalue_startend_val +
                  ",\"" + OT + BOILER_TEMPERATURE_SETPOINT + "\": " + payloadvalue_startend_val + String(tempBoilerSet, 1) + payloadvalue_startend_val +
                  ",\"" + OT + BOILER_CH_STATE + "\": " + payloadvalue_startend_val + String(status_CHActive ? payloadON : payloadOFF) + payloadvalue_startend_val +
                  ",\"" + OT + BOILER_SOFTWARE_CH_STATE_MODE + "\": \"" + String(boilermode) +
                  "\",\"" + OT + FLAME_STATE + "\": " + payloadvalue_startend_val + String(status_FlameOn ? payloadON : payloadOFF) + payloadvalue_startend_val +
                  ",\"" + OT + FLAME_LEVEL + "\": " + payloadvalue_startend_val + String(flame_level, 0) + payloadvalue_startend_val +
                  ",\"" + OT + FLAME_W + "\": " + payloadvalue_startend_val + String(flame_used_power,4) + payloadvalue_startend_val +
                  ",\"" + OT + FLAME_W_TOTAL + "\": " + payloadvalue_startend_val + String(flame_used_power_kwh,4) + payloadvalue_startend_val +
                  ",\"" + OT + TEMP_CUTOFF + "\": " + payloadvalue_startend_val + String(cutOffTemp, 1) + payloadvalue_startend_val +
                  "}").c_str(), mqtt_Retain); //"heat" : "off")    boilermode.c_str(),1);// ? "auto" : "heat" : "off",1); //heatingEnabled ? "1" : "0",1);  //"heat" : "off",1);

  mqttclient.publish(DIAG_TOPIC.c_str(),
                 ("{\"" + OT + DIAGS_OTHERS_FAULT + "\": " + payloadvalue_startend_val + String(status_Fault ? payloadON : payloadOFF) + payloadvalue_startend_val +
                  ",\"" + OT + DIAGS_OTHERS_DIAG + "\": " + payloadvalue_startend_val + String(status_Diagnostic ? payloadON : payloadOFF) + payloadvalue_startend_val +
                  ",\"" + OT + INTEGRAL_ERROR_GET_TOPIC + "\": " + payloadvalue_startend_val + String(ierr) + payloadvalue_startend_val +
                  "}").c_str(), mqtt_Retain); //"heat" : "off")



  publishhomeassistantconfig++; // zwiekszamy licznik wykonan wyslania mqtt by co publishhomeassistantconfigdivider wysłań wysłać autoconfig discovery dla homeassisatnt
  if (publishhomeassistantconfig % publishhomeassistantconfigdivider == 0)
  {
    mqttclient.setBufferSize(2048);

    // homeassistant/sensor/BB050B_OPENTHERM_OT10_lo/config = {"name":"Opentherm OPENTHERM OT10 lo","stat_t":"tele/tasmota_BB050B/SENSOR","avty_t":"tele/tasmota_BB050B/LWT","pl_avail":"Online","pl_not_avail":"Offline","uniq_id":"BB050B_OPENTHERM_OT10_lo","dev":{"ids":["BB050B"]},"unit_of_meas":" ","ic":"mdi:eye","frc_upd":true,"val_tpl":"{{value_json['OPENTHERM']['OT10']['lo']}}"} (retained) problem
    // 21:16:02.724 MQT: homeassistant/sensor/BB050B_OPENTHERM_OT10_hi/config = {"name":"Opentherm OPENTHERM OT10 hi","stat_t":"tele/tasmota_BB050B/SENSOR","avty_t":"tele/tasmota_BB050B/LWT","pl_avail":"Online","pl_not_avail":"Offline","uniq_id":"BB050B_OPENTHERM_OT10_hi","dev":{"ids":["BB050B"]},"unit_of_meas":" ","ic":"mdi:eye","frc_upd":true,"val_tpl":"{{value_json['OPENTHERM']['OT10']['hi']}}"} (retained)
    mqttclient.publish((DIAG_HABS_TOPIC + "_" + DIAGS_OTHERS_FAULT + "/config").c_str(), ("{\"name\":\"" + OT + DIAGS_OTHERS_FAULT + "\",\"uniq_id\": \"" + OT + DIAGS_OTHERS_FAULT + "\",\"stat_t\":\"" + DIAG_TOPIC + "\",\"payload_on\": " + payloadON + ",\"payload_off\": " + payloadOFF + ",\"val_tpl\":\"{{value_json." + OT + DIAGS_OTHERS_FAULT + "}}\",\"dev_cla\":\"problem\",\"unit_of_meas\": \" \",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    mqttclient.publish((DIAG_HABS_TOPIC + "_" + DIAGS_OTHERS_DIAG + "/config").c_str(), ("{\"name\":\"" + OT + DIAGS_OTHERS_DIAG + "\",\"uniq_id\": \"" + OT + DIAGS_OTHERS_DIAG + "\",\"stat_t\":\"" + DIAG_TOPIC + "\",\"payload_on\": " + payloadON + ",\"payload_off\": " + payloadOFF + ",\"val_tpl\":\"{{value_json." + OT + DIAGS_OTHERS_DIAG + "}}\",\"unit_of_meas\": \" \",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    mqttclient.publish((DIAG_HA_TOPIC + "_" + INTEGRAL_ERROR_GET_TOPIC + "/config").c_str(), ("{\"name\":\"" + OT + INTEGRAL_ERROR_GET_TOPIC + "\",\"uniq_id\": \"" + OT + INTEGRAL_ERROR_GET_TOPIC + "\",\"stat_t\":\"" + DIAG_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + INTEGRAL_ERROR_GET_TOPIC + "}}\",\"unit_of_meas\": \" \",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    mqttclient.publish((DIAG_HA_TOPIC + "_" + LOGS + "/config").c_str(), ("{\"name\":\"" + OT + LOGS + "\",\"uniq_id\": \"" + OT + LOGS + "\",\"stat_t\":\"" + LOG_GET_TOPIC + "\",\"val_tpl\":\"{{ value }}\",\"unit_of_meas\": \" \",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);

    mqttclient.publish((ROOM_OTHERS_HA_TOPIC + "_" + ROOM_OTHERS_TEMPERATURE + "/config").c_str(), ("{\"name\":\"" + OT + ROOM_OTHERS_TEMPERATURE + "\",\"uniq_id\": \"" + OT + ROOM_OTHERS_TEMPERATURE + "\",\"stat_t\":\"" + ROOM_OTHERS_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + ROOM_OTHERS_TEMPERATURE + "}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    mqttclient.publish((ROOM_OTHERS_HA_TOPIC + "_" + ROOM_OTHERS_TEMPERATURE_SETPOINT + "/config").c_str(), ("{\"name\":\"" + OT + ROOM_OTHERS_TEMPERATURE_SETPOINT + "\",\"uniq_id\": \"" + OT + ROOM_OTHERS_TEMPERATURE_SETPOINT + "\",\"stat_t\":\"" + ROOM_OTHERS_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + ROOM_OTHERS_TEMPERATURE_SETPOINT + "}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    mqttclient.publish((ROOM_OTHERS_HA_TOPIC + "_" + ROOM_OTHERS_PRESSURE + "/config").c_str(), ("{\"name\":\"" + OT + ROOM_OTHERS_PRESSURE + "\",\"uniq_id\": \"" + OT + ROOM_OTHERS_PRESSURE + "\",\"stat_t\":\"" + ROOM_OTHERS_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + ROOM_OTHERS_PRESSURE + "}}\",\"dev_cla\":\"pressure\",\"unit_of_meas\": \"hPa\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);

    mqttclient.publish((HOT_WATER_HA_TOPIC + "_" + HOT_WATER_TEMPERATURE + "/config").c_str(), ("{\"name\":\"" + OT + HOT_WATER_TEMPERATURE + "\",\"uniq_id\": \"" + OT + HOT_WATER_TEMPERATURE + "\",\"stat_t\":\"" + HOT_WATER_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + HOT_WATER_TEMPERATURE + "}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    mqttclient.publish((HOT_WATER_HA_TOPIC + "_" + HOT_WATER_TEMPERATURE_SETPOINT + "/config").c_str(), ("{\"name\":\"" + OT + HOT_WATER_TEMPERATURE_SETPOINT + "\",\"uniq_id\": \"" + OT + HOT_WATER_TEMPERATURE_SETPOINT + "\",\"stat_t\":\"" + HOT_WATER_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + HOT_WATER_TEMPERATURE_SETPOINT + "}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    mqttclient.publish((HOT_WATER_HABS_TOPIC + "_" + HOT_WATER_CH_STATE + "/config").c_str(), ("{\"name\":\"" + OT + HOT_WATER_CH_STATE + "\",\"uniq_id\": \"" + OT + HOT_WATER_CH_STATE + "\",\"stat_t\":\"" + HOT_WATER_TOPIC + "\",\"payload_on\": " + payloadON + ",\"payload_off\": " + payloadOFF + ",\"val_tpl\":\"{{value_json." + OT + HOT_WATER_CH_STATE + "}}\",\"dev_cla\":\"heat\",\"unit_of_meas\":\" \",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    mqttclient.publish((HOT_WATER_HABS_TOPIC + "_" + HOT_WATER_SOFTWARE_CH_STATE + "/config").c_str(), ("{\"name\":\"" + OT + HOT_WATER_SOFTWARE_CH_STATE + "\",\"uniq_id\": \"" + OT + HOT_WATER_SOFTWARE_CH_STATE + "\",\"stat_t\":\"" + HOT_WATER_TOPIC + "\",\"payload_on\": " + payloadON + ",\"payload_off\": " + payloadOFF + ",\"val_tpl\":\"{{value_json." + OT + HOT_WATER_SOFTWARE_CH_STATE + "}}\",\"dev_cla\":\"heat\",\"unit_of_meas\": \" \",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);

    mqttclient.publish((BOILER_HA_TOPIC + "_" + BOILER_TEMPERATURE + "/config").c_str(), ("{\"name\":\"" + OT + BOILER_TEMPERATURE + "\",\"uniq_id\": \"" + OT + BOILER_TEMPERATURE + "\",\"stat_t\":\"" + BOILER_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + BOILER_TEMPERATURE + "}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    mqttclient.publish((BOILER_HA_TOPIC + "_" + BOILER_TEMPERATURE_RET + "/config").c_str(), ("{\"name\":\"" + OT + BOILER_TEMPERATURE_RET + "\",\"uniq_id\": \"" + OT + BOILER_TEMPERATURE_RET + "\",\"stat_t\":\"" + BOILER_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + BOILER_TEMPERATURE_RET + "}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    mqttclient.publish((BOILER_HA_TOPIC + "_" + BOILER_TEMPERATURE_SETPOINT + "/config").c_str(), ("{\"name\":\"" + OT + BOILER_TEMPERATURE_SETPOINT + "\",\"uniq_id\": \"" + OT + BOILER_TEMPERATURE_SETPOINT + "\",\"stat_t\":\"" + BOILER_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + BOILER_TEMPERATURE_SETPOINT + "}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    mqttclient.publish((BOILER_HABS_TOPIC + "_" + BOILER_CH_STATE + "/config").c_str(), ("{\"name\":\"" + OT + BOILER_CH_STATE + "\",\"uniq_id\": \"" + OT + BOILER_CH_STATE + "\",\"stat_t\":\"" + BOILER_TOPIC + "\",\"payload_on\": " + payloadON + ",\"payload_off\": " + payloadOFF + ",\"val_tpl\":\"{{value_json." + OT + BOILER_CH_STATE + "}}\",\"dev_cla\":\"heat\",\"unit_of_meas\": \" \",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    mqttclient.publish((BOILER_HA_TOPIC + "_" + BOILER_SOFTWARE_CH_STATE_MODE + "/config").c_str(), ("{\"name\":\"" + OT + BOILER_SOFTWARE_CH_STATE_MODE + "\",\"uniq_id\": \"" + OT + BOILER_SOFTWARE_CH_STATE_MODE + "\",\"stat_t\":\"" + BOILER_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + BOILER_SOFTWARE_CH_STATE_MODE + "}}\",\"unit_of_meas\": \" \",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    mqttclient.publish((BOILER_HABS_TOPIC + "_" + FLAME_STATE + "/config").c_str(), ("{\"name\":\"" + OT + FLAME_STATE + "\",\"uniq_id\": \"" + OT + FLAME_STATE + "\",\"stat_t\":\"" + BOILER_TOPIC + "\",\"payload_on\": " + payloadON + ",\"payload_off\": " + payloadOFF + ",\"val_tpl\":\"{{value_json." + OT + FLAME_STATE + "}}\",\"dev_cla\":\"heat\",\"unit_of_meas\":\" \",\"ic\": \"mdi:fire\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    mqttclient.publish((BOILER_HA_TOPIC + "_" + FLAME_LEVEL + "/config").c_str(), ("{\"name\":\"" + OT + FLAME_LEVEL + "\",\"uniq_id\": \"" + OT + FLAME_LEVEL + "\",\"stat_t\":\"" + BOILER_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + FLAME_LEVEL + "}}\",\"dev_cla\":\"power\",\"unit_of_meas\":\"%\",\"ic\": \"mdi:fire\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    mqttclient.publish((BOILER_HA_TOPIC + "_" + FLAME_W + "/config").c_str(), ("{\"name\":\"" + OT + FLAME_W + "\",\"uniq_id\": \"" + OT + FLAME_W + "\",\"stat_t\":\"" + BOILER_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + FLAME_W + "}}\",\"dev_cla\":\"energy\",\"unit_of_meas\":\"kWh\",\"state_class\":\"measurement\",\"ic\": \"mdi:fire\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    mqttclient.publish((BOILER_HA_TOPIC + "_" + FLAME_W_TOTAL + "/config").c_str(), ("{\"name\":\"" + OT + FLAME_W_TOTAL + "\",\"uniq_id\": \"" + OT + FLAME_W_TOTAL + "\",\"stat_t\":\"" + BOILER_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + FLAME_W_TOTAL + "}}\",\"dev_cla\":\"energy\",\"unit_of_meas\":\"kWh\",\"state_class\":\"total_increasing\",\"ic\": \"mdi:fire\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);

    mqttclient.publish((BOILER_HA_TOPIC + "_" + TEMP_CUTOFF + "/config").c_str(), ("{\"name\":\"" + OT + TEMP_CUTOFF + "\",\"uniq_id\": \"" + OT + TEMP_CUTOFF + "\",\"stat_t\":\"" + BOILER_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + TEMP_CUTOFF + "}}\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);

    mqttclient.publish((HOT_WATER_HACLI_TOPIC + "_climate/config").c_str(), ("{\"name\":\"" + OT + "Hot Water\",\"uniq_id\": \"" + OT + "Hot_Water\", \
\"modes\":[\"off\",\"heat\"], \
\"icon\": \"mdi:water-pump\", \
\"current_temperature_topic\":\"" + HOT_WATER_TOPIC + "\", \
\"current_temperature_template\":\"{{value_json." + OT + HOT_WATER_TEMPERATURE + "}}\", \
\"temperature_command_topic\":\"" + TEMP_DHW_SET_TOPIC + "\", \
\"temperature_state_topic\":\"" + HOT_WATER_TOPIC + "\", \
\"temperature_state_template\":\"{{value_json." + OT + HOT_WATER_TEMPERATURE_SETPOINT + "}}\", \
\"temperature_unit\":\"C\", \
\"mode_state_topic\": \"" + HOT_WATER_TOPIC + "\", \
\"mode_state_template\": \"{{value_json." + OT + HOT_WATER_SOFTWARE_CH_STATE + "}}\", \
\"mode_command_topic\": \"" + STATE_DHW_SET_TOPIC + "\", \
\"mode_command_template\" : \"{% set values = { 'heat':'1', 'off':'0'} %}   {{ values[value] if value in values.keys() else '0' }}\", \
\"temp_step\": 0.5, \
\"precision\": 0.5, \
\"target_temp_step\": 0.5, \
\"min_temp\": " + oplo + ", \
\"max_temp\": " + ophi + ", \
\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    mqttclient.publish((BOILER_HACLI_TOPIC + "_climate/config").c_str(), ("{\"name\":\"" + OT + "Boiler CO\",\"uniq_id\": \"" + OT + "Boiler_CO\", \
\"modes\":[\"off\",\"heat\",\"auto\"], \
\"icon\": \"mdi:water-pump\", \
\"current_temperature_topic\":\"" + BOILER_TOPIC + "\", \
\"current_temperature_template\":\"{{value_json." + OT + BOILER_TEMPERATURE + "}}\", \
\"temperature_command_topic\":\"" + TEMP_SETPOINT_SET_TOPIC + "\", \
\"temperature_state_topic\":\"" + BOILER_TOPIC + "\", \
\"temperature_state_template\":\"{{value_json." + OT + BOILER_TEMPERATURE_SETPOINT + "}}\", \
\"temperature_unit\":\"C\", \
\"mode_state_topic\": \"" + BOILER_TOPIC + "\", \
\"mode_state_template\": \"{{value_json." + OT + BOILER_SOFTWARE_CH_STATE_MODE + "}}\", \
\"mode_command_topic\": \"" + MODE_SET_TOPIC + "\", \
\"mode_command_template\" : \"{% set values = { 'auto':'2', 'heat':'1', 'off':'0'} %}   {{ values[value] if value in values.keys() else '0' }}\", \
\"temp_step\": 0.5, \
\"precision\": 0.5, \
\"target_temp_step\": 0.5, \
\"min_temp\": " + opcolo + ", \
\"max_temp\": " + opcohi + ", \
\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    mqttclient.publish((ROOM_OTHERS_HACLI_TOPIC + "_climate/config").c_str(), ("{\"name\":\"" + OT + "Boiler RoomTemp Control CO\",\"uniq_id\": \"" + OT + "Boiler_RoomTemp_Control_CO\", \
\"modes\":[\"off\",\"heat\",\"auto\"], \
\"icon\": \"mdi:water-pump\", \
\"current_temperature_topic\":\"" + ROOM_OTHERS_TOPIC + "\", \
\"current_temperature_template\":\"{{value_json." + OT + ROOM_OTHERS_TEMPERATURE + "}}\", \
\"temperature_command_topic\":\"" + ROOM_TEMP_SET_TOPIC + "\", \
\"temperature_state_topic\":\"" + ROOM_OTHERS_TOPIC + "\", \
\"temperature_state_template\":\"{{value_json." + OT + ROOM_OTHERS_TEMPERATURE_SETPOINT + "}}\", \
\"temperature_unit\":\"C\", \
\"mode_state_topic\": \"" + BOILER_TOPIC + "\", \
\"mode_state_template\": \"{{value_json." + OT + BOILER_SOFTWARE_CH_STATE_MODE + "}}\", \
\"mode_command_topic\": \"" + MODE_SET_TOPIC + "\", \
\"mode_command_template\" : \"{% set values = { 'auto':'2', 'heat':'1', 'off':'0'} %}   {{ values[value] if value in values.keys() else '0' }}\", \
\"temp_step\": 0.5, \
\"precision\": 0.5, \
\"target_temp_step\": 0.5, \
\"min_temp\": " + roomtemplo + ", \
\"max_temp\": " + roomtemphi + ", \
\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    mqttclient.publish((ROOM_OTHERS_HACLI_TOPIC + "cutoff_climate/config").c_str(), ("{\"name\":\"" + OT + "CutoffTemp\",\"uniq_id\": \"" + OT + "CutoffTemp\", \
\"modes\":\"\", \
\"icon\": \"mdi:water-pump\", \
\"current_temperature_topic\":\"" + NEWS_GET_TOPIC + "\", \
\"current_temperature_template\":\"{{value}}\", \
\"temperature_command_topic\":\"" + TEMP_CUTOFF_SET_TOPIC + "\", \
\"temperature_state_topic\":\"" + BOILER_TOPIC + "\", \
\"temperature_state_template\":\"{{value_json." + OT + TEMP_CUTOFF + "}}\", \
\"temperature_unit\":\"C\", \
\"temp_step\": 0.5, \
\"precision\": 0.5, \
\"target_temp_step\": 0.5, \
\"min_temp\": " + cutofflo + ", \
\"max_temp\": " + cutoffhi + ", \
\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
  }

  String tempSource = (millis() - lastTempSet > extTempTimeout_ms)
                          ? "(internal sensor)"
                          : "(external sensor)";
  sprintf(log_chars,"Current temperature 18B20: %s °C %s", String(roomtemp).c_str(), tempSource.c_str());
  log_message(log_chars);
}

void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
  #ifdef enableMQTT
  const String topicStr(topic);

  String payloadStr = convertPayloadToStr(payload, length);
  payloadStr.trim();

  if (topicStr == ROOMS_F1_GET_TOPIC) //topic min temp and max setpoint from floor 1
  {
    String ident = "Floor1temp ";
 //   WebSerial.println("Payload: " + String(payloadStr));
    if (PayloadtoValidFloatCheck(getJsonVal(payloadStr,roomF1temp_json)))  //wrong value are displayed in function
    {
      floor1_temp = PayloadtoValidFloat(getJsonVal(payloadStr,roomF1temp_json), true, roomtemplo, roomtemphi);
      sprintf(log_chars, "%s set to: %s", ident.c_str(), String(floor1_temp).c_str());
      log_message(log_chars);
      receivedmqttdata = true;
    } else {
      sprintf(log_chars, "%s is not a valid number, ignoring..., payloadStr: %s", ident.c_str(), payloadStr.c_str());
      log_message(log_chars);
    }
    if (PayloadtoValidFloatCheck(getJsonVal(payloadStr,roomF1tempset_json)))  //wrong value are displayed in function
    {
      floor1_tempset = PayloadtoValidFloat(getJsonVal(payloadStr,roomF1tempset_json),true, roomtemplo, roomtemphi);
      sprintf(log_chars, "%s Setpoint set to: %s", ident.c_str(), String(floor1_tempset).c_str());
      log_message(log_chars);
      receivedmqttdata = true;
    } else {
      sprintf(log_chars, "%s Setpoint is not a valid number, ignoring..., payloadStr: %s", ident.c_str(), payloadStr.c_str());
      log_message(log_chars);
    }
  }
  else if (topicStr == ROOMS_F2_GET_TOPIC) //topic min temp and max setpoint from floor 1
  {
    String ident = "Floor2temp ";
 //   WebSerial.println("Payload: " + String(payloadStr));
    if (PayloadtoValidFloatCheck(getJsonVal(payloadStr,roomF2temp_json)))  //wrong value are displayed in function
    {
      floor2_temp = PayloadtoValidFloat(getJsonVal(payloadStr,roomF2temp_json), true, roomtemplo, roomtemphi);
      sprintf(log_chars, "%s set to: %s", ident.c_str(), String(floor2_temp).c_str());
      log_message(log_chars);
      receivedmqttdata = true;
    } else {
      sprintf(log_chars, "%s Setpoint is not a valid number, ignoring..., payloadStr: %s", ident.c_str(), payloadStr.c_str());
      log_message(log_chars);
    }
    if (PayloadtoValidFloatCheck(getJsonVal(payloadStr,roomF2tempset_json)))  //wrong value are displayed in function
    {
      floor2_tempset = PayloadtoValidFloat(getJsonVal(payloadStr,roomF2tempset_json),true, roomtemplo, roomtemphi);
      sprintf(log_chars, "%s Setpoint set to: %s", ident.c_str(), String(floor2_tempset).c_str());
      log_message(log_chars);
      receivedmqttdata = true;
    } else {
      sprintf(log_chars, "%s Setpoint is not a valid number, ignoring..., payloadStr: %s", ident.c_str(), payloadStr.c_str());
      log_message(log_chars);
    }
  } else
  if (topicStr == NEWS_GET_TOPIC)               //NEWS averange temp -outside temp
  {
    String ident = "NEWS temp ";
    if (PayloadtoValidFloatCheck(getJsonVal(payloadStr,NEWStemp_json)))           //invalid val is displayed in funct
    {
      temp_NEWS = PayloadtoValidFloat(getJsonVal(payloadStr,NEWStemp_json),true);     //true to get output to serial and webserial
      sprintf(log_chars, "%s updated from MQTT to: %s", ident.c_str(), String(temp_NEWS).c_str());
      log_message(log_chars);
      lastNEWSSet = millis();
      temp_NEWS_count = 0;
//      receivedmqttdata = true;    //makes every second run mqtt send and influx
    }
  }
  else if (topicStr == ROOM_TEMP_SET_TOPIC)           // Rooms autosetp.roomtemp for auto mode
  {
    String ident = "Rooms Current roomtemp ";
    if (PayloadtoValidFloatCheck(payloadStr))  //wrong value are displayed in function
    {
      sp = PayloadtoValidFloat(payloadStr, true, roomtemplo, roomtemphi);
      sprintf(log_chars, "%s set to: %s", ident.c_str(), String(sp).c_str());
      log_message(log_chars);
      receivedmqttdata = true;
    } else {
      sprintf(log_chars, "%s is not a valid number, ignoring..., payloadStr: %s", ident.c_str(), payloadStr.c_str());
      log_message(log_chars);
    }
  }
  else if (topicStr == TEMP_SETPOINT_SET_TOPIC)      //Room Target sp for automode
  {
    String ident = "Room Target sp ";
   if (PayloadtoValidFloatCheck(payloadStr))  //wrong value are displayed in function
    {
      tempBoilerSet = PayloadtoValidFloat(payloadStr, true, roomtemplo, roomtemphi);
      op_override = tempBoilerSet; // when no auto heating then this is temp to heat CO
      sprintf(log_chars, "%s set to: %s", ident.c_str(), String(tempBoilerSet).c_str());
      log_message(log_chars);
      receivedmqttdata = true;
    } else {
      sprintf(log_chars, "%s is not a valid number, ignoring..., payloadStr: %s", ident.c_str(), payloadStr.c_str());
      log_message(log_chars);
    }
  }
  else if (topicStr == MODE_SET_TOPIC)              //mode set topic
  {
    String ident = "Mode Set ";
    payloadStr.toUpperCase();
    if (PayloadStatus(payloadStr, true))
    {
      heatingEnabled = true;
      automodeCO = false;
      tempBoilerSet = op_override;
      receivedmqttdata = true;
      sprintf(log_chars, "%s to: CO mode  %s", ident.c_str(), String(payloadStr).c_str());
      log_message(log_chars);
    }
    else if (PayloadStatus(payloadStr, false))
    {
      heatingEnabled = false;
      automodeCO = false;
      receivedmqttdata = true;
      sprintf(log_chars, "%s to: CO mode  %s", ident.c_str(), String(payloadStr).c_str());
      log_message(log_chars);
    }
    else if (payloadStr == "AUTO" or payloadStr == "2")
    {
      automodeCO = true;
      receivedmqttdata = true;
      sprintf(log_chars, "%s to: CO mode  %s", ident.c_str(), String(payloadStr).c_str());
      log_message(log_chars);
    } else {
      sprintf(log_chars, "%s to: Unknown mode  %s", ident.c_str(), String(payloadStr).c_str());
      log_message(log_chars);
    }
  }
  else if (topicStr == TEMP_DHW_SET_TOPIC)    // dhwTarget
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
  else if (topicStr == STATE_DHW_SET_TOPIC)   // enableHotWater
  {
    String ident ="DHW State enableHotWater ";
    receivedmqttdata = true;
    if (PayloadStatus(payloadStr, true)) {enableHotWater = true;}
    else if (PayloadStatus(payloadStr, false)) {enableHotWater = false;}
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
  else if (topicStr == TEMP_CUTOFF_SET_TOPIC)         //cutOffTemp
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
 else
//CO Pump Status #define COPumpStatus_json "CO0_boilerroom_pump2CO"
//#define WaterPumpStatus_json "CO0_boilerroom_pump1Water"
  if (topicStr == COPUMP_GET_TOPIC)                                                                   //external CO Pump Status
  {
    String ident = "Wood/coax CO Pump Status ";
    receivedmqttdata = true;
    if (PayloadStatus(getJsonVal(payloadStr,COPumpStatus_json), true)) {CO_PumpWorking = true;}
    else if (PayloadStatus(getJsonVal(payloadStr,COPumpStatus_json), false)) {CO_PumpWorking = false;}
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
  }  else
//CO Pump Status #define COPumpStatus_json "CO0_boilerroom_pump2CO"
//#define WaterPumpStatus_json "CO0_boilerroom_pump1Water"
  if (topicStr == COPUMP_GET_TOPIC)                                                                   //external CO Pump Status
  {
    String ident = "Wood/coax Water Pump Status ";
    receivedmqttdata = true;
    if (PayloadStatus(getJsonVal(payloadStr,WaterPumpStatus_json), true)) {Water_PumpWorking = true;}
    else if (PayloadStatus(getJsonVal(payloadStr,WaterPumpStatus_json), false)) {Water_PumpWorking = false;}
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
  else if (topicStr == SETPOINT_OVERRIDE_SET_TOPIC)  //op_override -think unused
  {
    String ident = "Setpoint override op_override ";
    if (PayloadtoValidFloatCheck(payloadStr))  //wrong value are displayed in function
    {
      op_override = PayloadtoValidFloat(payloadStr, true); //true to avoid duplicated messages to serial and webserial
      sprintf(log_chars, "%s to: %s", ident.c_str(), String(op_override).c_str());
      log_message(log_chars);
      receivedmqttdata = true;
    } else {
      sprintf(log_chars, "%s is not a valid number, ignoring... paload: %s", ident.c_str(), payloadStr.c_str());
      log_message(log_chars);
    }
  }
  else if (topicStr == SETPOINT_OVERRIDE_RESET_TOPIC)         //think not used
  {
    String ident = "Setpoint override reset ";
    lastSpSet = 0;
    sprintf(log_chars, "%s , paloadStr: %s", ident.c_str(), String(payloadStr).c_str());
    log_message(log_chars);
  }
  else
  {
    sprintf(log_chars, "Unknown topic: %s", String(topic).c_str());
    log_message(log_chars);
    return;
  }
  #endif
}

void mqtt_reconnect_subscribe_list()
{
      mqttclient.subscribe(TEMP_SETPOINT_SET_TOPIC.c_str());
      mqttclient.subscribe(MODE_SET_TOPIC.c_str());
      mqttclient.subscribe(ROOM_TEMP_SET_TOPIC.c_str());
      mqttclient.subscribe(TEMP_DHW_SET_TOPIC.c_str());
      mqttclient.subscribe(STATE_DHW_SET_TOPIC.c_str());
      mqttclient.subscribe(SETPOINT_OVERRIDE_SET_TOPIC.c_str());
      mqttclient.subscribe(SETPOINT_OVERRIDE_RESET_TOPIC.c_str());
      mqttclient.subscribe(NEWS_GET_TOPIC.c_str());
      mqttclient.subscribe(COPUMP_GET_TOPIC.c_str());
      mqttclient.subscribe(TEMP_CUTOFF_SET_TOPIC.c_str());
      mqttclient.subscribe(ROOMS_F1_GET_TOPIC.c_str());
      mqttclient.subscribe(ROOMS_F2_GET_TOPIC.c_str());
}

void setup()
{
  Serial.begin(74880);

  Serial.println(F("Starting... ..."));
  getFreeMemory();
  #ifdef doubleResDet
  // double reset detect from start
  doubleResetDetect();
  #endif
  if (loadConfig())
  {
    Serial.println(F("Config loaded:"));
    Serial.println(CONFIGURATION.version);
    Serial.println(CONFIGURATION.ssid);
    Serial.println(CONFIGURATION.pass);
    Serial.println(CONFIGURATION.mqtt_server);
    Serial.println(CONFIGURATION.COPUMP_GET_TOPIC);
    Serial.println(CONFIGURATION.NEWS_GET_TOPIC);
  }
  else
  {
    Serial.println(F("Config not loaded!"));
    SaveConfig(); // overwrite with the default settings
  }

  MainCommonSetup();
  ot.begin(handleInterrupt);

  // Init DS18B20 sensor
  sensors.begin();
  sensors.requestTemperatures();
  sensors.setWaitForConversion(false); // switch to async mode
  roomtemp = sensors.getTempCByIndex(0);
  roomtemp_last = roomtemp;

  ts = millis();
  lastTempSet = -extTempTimeout_ms;


  #ifdef debug
  Serial.println("end setup....");
  //    SaveConfig();
#endif

}




void loop()
{
  MainCommonLoop();
  unsigned long now = millis() + 100; // TO AVOID compare -2>10000 which is true ??? why?
  // check mqtt is available and connected in other case check values in api.


  if ((now - lastUpdate) > statusUpdateInterval_ms)
  {
    lastUpdate = now;
    opentherm_update_data(lastUpdatemqtt); // According OpenTherm Specification from Ihnor Melryk Master requires max 1s interval communication -przy okazji wg czasu update mqtt zrobie odczyt dallas
  updateDatatoWWW();
  }
  if (status_FlameOn) {
    unsigned long nowtime = millis();
    float boiler_power =0;

    if (retTemp<boiler_50_30_ret) boiler_power=boiler_50_30; else boiler_power=boiler_80_60;
    double boilerpower = boiler_power*(flame_level/100); //kW
    double time_to_hour = (nowtime-start_flame_time)/(double(hour_s)*1000);
    flame_used_power += boilerpower*time_to_hour/1000;
    flame_used_power_kwh += boilerpower*time_to_hour/1000;
    sprintf(log_chars,"nowtime: %s, BoilerPower: %s, time_to_hour: %s, flame_used_power bp*time/1k/1k: %s", String(nowtime).c_str(), String(boilerpower,6).c_str(), String(time_to_hour).c_str(), String(flame_used_power,6).c_str());
    log_message(log_chars);

    // WebSerial.print(String(start_flame_time));
    // WebSerial.print(": millis()-start_flame_time "+String((millis()-start_flame_time),10));
    // WebSerial.print(": ((millis()-start_flame_time)/1000)/hour_s "+String((((millis()-start_flame_time)/1000)/(double(hour_s))),10));
    // WebSerial.print(": time_to_hour "+String(time_to_hour,10));
    // WebSerial.print(": boiler_power "+String(boiler_power,4));
    // WebSerial.print(", boilerpower "+String(boilerpower,4));
    // WebSerial.print(": flame_used_power "+String(flame_used_power,4));
    // WebSerial.println(": boil_power_kwh "+String(boilerpower*time_to_hour/1000,10));
    // WebSerial.println(": flame_used_power_kwh "+String(flame_used_power_kwh,10));
    start_flame_time = nowtime;
  }
  if (((now - lastUpdatemqtt) > mqttUpdateInterval_ms) or (receivedmqttdata == true))
  {
    lastUpdatemqtt = now;
    roomtemp=getTemp();
  }
  //#define abs(x) ((x)>0?(x):-(x))
  if (((now - lastNEWSSet) > (temp_NEWS_interval_reduction_time_ms*2)) and 1==0) //disable for now
  { // at every 0,5hour lower temp NEWS when no communication why -2>1800000 is true ???
    sprintf(log_chars,"nowtime: %s, lastNEWSSet: %s, temp_NEWS_interval_reduction_time_ms: %s", String(now).c_str(), String(lastNEWSSet,6).c_str(), String(temp_NEWS_interval_reduction_time_ms).c_str());
    log_message(log_chars);
    lastNEWSSet = now;
    temp_NEWS_count++;
    if (temp_NEWS > cutOffTemp)
    {
      temp_NEWS = temp_NEWS - temp_NEWS * 0.05;
      sprintf(log_chars,"Lowering by 5%% temp_NEWS (no communication) -after 10times execute every 30minutes lowered temp NEWS from: %s, to: %s", String(temp_NEWS).c_str(), String(temp_NEWS - temp_NEWS * 0.05).c_str());
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
  }

  static bool failFlag = false;
  bool fail = now - lastTempSet > extTempTimeout_ms && now - lastSpSet > spOverrideTimeout_ms + now;
  if (fail)
  {
    if (!failFlag)
    {
      failFlag = true;
      sprintf(log_chars, "Neither temperature nor setpoint provided, setting heating water to: %s", String(noCommandSpOverride).c_str());
      log_message(log_chars);
    }

    lastSpSet = millis();
    op_override = noCommandSpOverride;
  }
  else
  {
    failFlag = false;
  }
  // webserver.handleClient(); //handle http requests
}
