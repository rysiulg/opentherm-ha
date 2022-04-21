/*************************************************************
  This example runs directly on ESP8266 chip.

  Please be sure to select the right ESP8266 module
  in the Tools -> Board -> WeMos D1 Mini

  Adjust settings in Config.h before run
 *************************************************************/

//#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PubSubClient.h>
#include <OpenTherm.h>
#include "config.h"
//#include <ArduinoHA.h>  //HomeAssistant
#include <WebSerial.h>

#include <WiFiClient.h>

// for ota
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
//#include <ESPAsyncUDP.h>
//#include <ESPAsyncDNSServer.h>

AsyncWebServer webserver(wwwport);
// for ota webs

const unsigned long extTempTimeout_ms = 180 * 1000,
                    statusUpdateInterval_ms = 0.9 * 1000,
                    spOverrideTimeout_ms = 180 * 1000,
                    mqttUpdateInterval_ms = 16 * 1000,
                    temp_NEWS_interval_reduction_time_ms = 30 * 60 * 1000, // time to laps between temp_NEWS reduction by 5%
                    mqtt_offline_reconnect_after_ms = 15 * 60 * 1000,      // time when mqtt is offline to wait for next reconnect (15minutes)
                    save_config_every = 15 * 60 * 1000,                    // time saveing config values in eeprom (15minutes)
                    WiFiinterval = 30 * 1000;

// upper and lower bounds on heater level
const float ophi = 65,               // upper max heat water
            oplo = 30,               // lower min heat water
            opcohi = 60,             // upper max heat boiler to CO
            opcolo = oplo,           // lower min heat boiler to CO
            cutoffhi = 20,           // upper max cut-off temp above is heating CO disabled -range +-20
            cutofflo = -cutoffhi,    // lower min cut-off temp above is heating CO disabled
            roomtemphi = 25,         // upper max to set of room temperature
            roomtemplo = 15,         // lower min to set of room temperature
            noCommandSpOverride = 32; //heating water temperature for fail mode (no external temp provided) for co

unsigned int runNumber = 0, // count of restarts
             publishhomeassistantconfig = 4,                               // licznik wykonan petli -strat od 0
             publishhomeassistantconfigdivider = 5;                        // publishhomeassistantconfig % publishhomeassistantconfigdivider -publikacja gdy reszta z dzielenia =0 czyli co te ilosc wykonan petli opoznionej update jest wysylany config

float dhwTarget = 51, // domyslna temperatura docelowa wody uzytkowej
      sp = 20.5,       // set point roomtemp
      roomtemp = 21,     // current temperature
      roomtemp_last = 0, // prior temperature
      ierr = 25,         // integral error
      dt = 0,            // time between measurements
      //       op = 0, //PID controller output
      retTemp = 0,                         // return temperature
      temp_NEWS = 0,                       // avg temperatura outside -getting this from mqtt topic as averange from 4 sensors North West East South
      cutOffTemp = 2,                      // outside temp setpoint to cutoff heating co. CO heating is disabled if outside temp (temp_NEWS) is above this value
      op_override = noCommandSpOverride, // boiler tempset on heat mode
      flame_level = 0,
      tempBoiler = 0,
      tempBoilerSet = op_override, // temp boiler set -mainly used in auto mode and for display on www actual temp
      tempCWU = 0,
      pressure = 0;


String LastboilerResponseError;

int temp_NEWS_count = 0,
    mqtt_offline_retrycount = 0,
    mqtt_offline_retries = 10; // retries to mqttconnect before timewait

unsigned long ts = 0, new_ts = 0, // timestamp
              lastUpdate = 0,
              lastUpdatemqtt = 0,
              lastTempSet = 0,
              lastcutOffTempSet = 0,
              lastNEWSSet = 0,
              lastmqtt_reconnect = 0,
              lastSaveConfig = 0,
              lastSpSet = 0,
              WiFipreviousMillis = 0,
              start_flame_time = 0;    //Flame Starts work

float    flame_used_power = 0;         //for count powerkWh between samples (boiler rated*flame power) use integer end when div use % do get decimal value

bool heatingEnabled = true,
     enableHotWater = true,
     CO_PumpWorking = false, // value from mqtt -if set co heating is disabled -second heating engine is working (in my case Wood/Carbon heater)
     automodeCO = false,     // tryb automatyczny zalezny od temp zewnetrznej i pracy pompy CO kotla weglowego
     receivedmqttdata = false,
     tmanual = false,
     status_Fault,
     status_CHActive,
     status_WaterActive,
     status_FlameOn,
     status_Cooling,
     status_Diagnostic;

OneWire oneWire(ROOM_TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);
OpenTherm ot(OT_IN_PIN, OT_OUT_PIN);
WiFiClient espClient;
PubSubClient client(espClient);

// ESP8266WebServer server(80);

#include "websrv_ota.h"

void recvMsg(uint8_t *data, size_t len)
{ // for WebSerial
  WebSerial.println("Received Data...");
  String d = "";
  for (int i = 0; i < len; i++)
  {
    d += char(data[i]);
  }
  WebSerial.println("Received: " + String(d));
  if (d == "ON")
  {
    //  digitalWrite(LED, HIGH);
  }
  if (d == "OFF")
  {
    //  digitalWrite(LED, LOW);
  }
  if (d == "RESTART")
  {
    WebSerial.println(F("OK. Restarting... by command..."));
    restart();
  }
  if (d == "RECONNECT")
  {
    reconnect();
  }
  if (d == "ROOMTEMP+")
  {
    WebSerial.print("Change t from: " + String(roomtemp));
    roomtemp = roomtemp + 1;
    tmanual = true;
    lastTempSet = millis();
    WebSerial.print(" to: " + String(roomtemp));
  }
  if (d == "ROOMTEMP-")
  {
    WebSerial.print("Change t from: " + String(roomtemp));
    roomtemp = roomtemp - 1;
    lastTempSet = millis();
    tmanual = true;
    WebSerial.print(" to: " + String(roomtemp));
  }
  if (d == "ROOMTEMP0")
  {
    WebSerial.print("Change t from: " + String(roomtemp));
    tmanual = false;
    lastTempSet = millis();
    WebSerial.print(" to: " + String(roomtemp));
  }
  if (d == "SAVE")
  {
    WebSerial.println(F("Saving config to EEPROM memory by command..."));
    WebSerial.println("Size CONFIG: " + String(sizeof(CONFIGURATION)));
    saveConfig();
  }
  if (d == "RESET-CONFIG")
  {
    WebSerial.println(F("RESET config to DEFAULT VALUES and restart..."));
    WebSerial.println("Size CONFIG: " + String(sizeof(CONFIGURATION)));
    CONFIGURATION.version[0] = 'R';
    CONFIGURATION.version[1] = 'E';
    CONFIGURATION.version[2] = 'S';
    CONFIGURATION.version[3] = 'E';
    CONFIGURATION.version[4] = 'T';
    saveConfig();
    restart();
  }
  if (d == "LOGCONFIG")
  {
    WebSerial.println("Saved CONFIG:");
    WebSerial.println("COPUMP_GET_TOPIC " + String(CONFIGURATION.COPUMP_GET_TOPIC));

    WebSerial.println("heatingEnabled " + String(CONFIGURATION.heatingEnabled));
    WebSerial.println("enableHotWater " + String(CONFIGURATION.enableHotWater));
    WebSerial.println("automodeCO " + String(CONFIGURATION.automodeCO));
    WebSerial.println("tempBoilerSet " + String(CONFIGURATION.tempBoilerSet));
    WebSerial.println("sp: " + String(CONFIGURATION.sp));
    WebSerial.println("cutOffTemp " + String(CONFIGURATION.cutOffTemp));
    WebSerial.println("op_override" + String(CONFIGURATION.op_override));
    WebSerial.println("dhwTarget " + String(CONFIGURATION.dhwTarget));
    WebSerial.println("roomtemp " + String(CONFIGURATION.roomtemp));
    WebSerial.println("temp_NEWS " + String(CONFIGURATION.temp_NEWS));
    WebSerial.println("ssid " + String(CONFIGURATION.ssid));
    WebSerial.println("pass " + String(CONFIGURATION.pass));
    WebSerial.println("mqtt_server " + String(CONFIGURATION.mqtt_server));
    WebSerial.println("mqtt_user " + String(CONFIGURATION.mqtt_user));
    WebSerial.println("mqtt_password " + String(CONFIGURATION.mqtt_password));
    WebSerial.println("mqtt_port " + String(CONFIGURATION.mqtt_port));

    WebSerial.println("NEWS_GET_TOPIC " + String(CONFIGURATION.NEWS_GET_TOPIC));
    WebSerial.println("Size CONFIG: " + String(sizeof(CONFIGURATION)));
  }
  if (d == "CO")
  {
    // espClient.connect("esp-b2c08e/dallThermometerS");
    client.disconnect();

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
    WebSerial.println(F("KOMENDY: RESTART, RECONNECT, ROOMTEMP+/-, ROOMTEMP0, SAVE, LOGCONFIG, RESET-CONFIG"));
  }
}

void IRAM_ATTR handleInterrupt()
{
  ot.handleInterrupt();
}

float getTemp()
{
  unsigned long now = millis();
  if ((now - lastTempSet) > extTempTimeout_ms)
  {
    lastTempSet = now;
    WebSerial.print(F("Update dallas temp."));
    return sensors.getTempCByIndex(0);
  }
  else
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
#ifdef debug
  Serial.println("sp=" + String(sp) + " pv=" + String(pv) + " dt=" + String(dt) + " op=" + String(op) + " P=" + String(P) + " I=" + String(I));
#endif
  if (publishhomeassistantconfig % publishhomeassistantconfigdivider == 0)
    WebSerial.println("sp=" + String(sp) + " pv=" + String(pv) + " dt=" + String(dt) + " op=" + String(op) + " P=" + String(P) + " I=" + String(I) + " tNEWS=" + String(temp_NEWS));

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

  unsigned long response = ot.setBoilerStatus(heatingEnabled, enableHotWater, enableCooling); // enableOutsideTemperatureCompensation
  OpenThermResponseStatus responseStatus = ot.getLastResponseStatus();
  if (responseStatus != OpenThermResponseStatus::SUCCESS)
  {
    String msg = "Error: Invalid boiler response " + String(response, HEX);
#ifdef debug
    Serial.println(msg);
#endif
    WebSerial.println(msg);
    LastboilerResponseError = msg;
  }
  ot.setDHWSetpoint(dhwTarget);

  if (tmanual == false)
    roomtemp = getTemp();

  if (roomtemp == -127)
    roomtemp = sp;
  if (roomtemp_last == -127)
    roomtemp_last = sp;
  float op = 0;
  if (responseStatus == OpenThermResponseStatus::SUCCESS)
  {
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
  }

  roomtemp_last = roomtemp;

  status_Fault = ot.isFault(response);
  status_CHActive = ot.isCentralHeatingActive(response);
  status_WaterActive = ot.isHotWaterActive(response);
  bool status_flame_tmp = status_FlameOn;
  status_FlameOn = ot.isFlameOn(response);
  if (status_flame_tmp != status_FlameOn) {
    status_FlameOn? start_flame_time=millis() : start_flame_time=0;  //After change flame status If flame is on get timer, else reset timer
  }
  status_Cooling = ot.isCoolingActive(response);
  status_Diagnostic = ot.isDiagnostic(response);
  flame_level = ot.getModulation();
  tempBoiler = ot.getBoilerTemperature();
  tempCWU = ot.getDHWTemperature();
  retTemp = ot.getReturnTemperature();
  pressure = ot.getPressure();

  if (millis() - mqttdallas > mqttUpdateInterval_ms)
  {
    sensors.requestTemperatures(); // async temperature request
    WebSerial.println(F("Request Dallas temperatures..."));
  }
}

// This function  sends data to MQTT .
void updateData()
{
  const String payloadvalue_startend_val = ""; // value added before and after value send to mqtt queue
  const String payloadON = "1";
  const String payloadOFF = "0";
  String boilermode;

  client.publish(LOG_GET_TOPIC.c_str(), LastboilerResponseError.c_str());

  if (automodeCO)
    boilermode = "auto";
  else if (heatingEnabled)
    boilermode = "heat";
  else
    boilermode = "off";

    unsigned long flame_elapsed_time = (millis()-start_flame_time);
    String flame_used_energy=String(((flame_used_power)*1000)/1,4);  //unit W
    WebSerial.print(String(millis())+": flame_used_power Wh: "); WebSerial.println(String(flame_used_power));
    WebSerial.print(String(millis())+": flame_elapsed_time: "); WebSerial.println(String(flame_elapsed_time));
    WebSerial.print(String(millis())+": flame_W used: "); WebSerial.println(String(flame_used_energy));
    WebSerial.println("Flame level: "+String(flame_level));
    flame_used_power=0;
    start_flame_time=0;






  client.setBufferSize(512);
  if (status_Fault)
    WebSerial.println("Błąd: " + String(status_Fault ? "on" : "off"));
  if (status_CHActive)
    WebSerial.println("Status_CHActive: " + String(status_CHActive ? "on" : "off"));
  if (status_WaterActive)
    WebSerial.println("Status_WaterActive: " + String(status_WaterActive ? "on" : "off"));
  if (enableHotWater)
    WebSerial.println("EnableHW: " + String(enableHotWater ? "on" : "off"));
  if (status_FlameOn)
    WebSerial.println("Status_FlameOn: " + String(status_FlameOn ? "on" : "off"));
  if (status_Cooling)
    WebSerial.println("Status_Cooling: " + String(status_Cooling ? "on" : "off"));
  if (status_Diagnostic)
    WebSerial.println("Status_Diagnostic: " + String(status_Diagnostic ? "on" : "off"));

  client.publish(ROOM_OTHERS_TOPIC.c_str(),
                 ("{\"" + OT + ROOM_OTHERS_TEMPERATURE + "\": " + payloadvalue_startend_val + String(roomtemp) + payloadvalue_startend_val +
                  ",\"" + OT + ROOM_OTHERS_TEMPERATURE_SETPOINT + "\": " + payloadvalue_startend_val + String(sp) + payloadvalue_startend_val +
                  ",\"" + OT + ROOM_OTHERS_PRESSURE + "\": " + payloadvalue_startend_val + String(pressure) + payloadvalue_startend_val +
                  "}").c_str(), mqtt_Retain); //"heat" : "off")

  client.publish(HOT_WATER_TOPIC.c_str(),
                 ("{\"" + OT + HOT_WATER_TEMPERATURE + "\": " + payloadvalue_startend_val + String(tempCWU) + payloadvalue_startend_val +
                  ",\"" + OT + HOT_WATER_TEMPERATURE_SETPOINT + "\": " + payloadvalue_startend_val + String(dhwTarget, 1) + payloadvalue_startend_val +
                  ",\"" + OT + HOT_WATER_CH_STATE + "\": " + payloadvalue_startend_val + String(status_WaterActive ? payloadON : payloadOFF) + payloadvalue_startend_val +
                  ",\"" + OT + HOT_WATER_SOFTWARE_CH_STATE + "\": \"" + String(enableHotWater ? "heat" : "off") + "\"" +
                  "}").c_str(), mqtt_Retain); //"heat" : "off")

  client.publish(BOILER_TOPIC.c_str(),
                 ("{\"" + OT + BOILER_TEMPERATURE + "\": " + payloadvalue_startend_val + String(tempBoiler) + payloadvalue_startend_val +
                  ",\"" + OT + BOILER_TEMPERATURE_RET + "\": " + payloadvalue_startend_val + String(retTemp) + payloadvalue_startend_val +
                  ",\"" + OT + BOILER_TEMPERATURE_SETPOINT + "\": " + payloadvalue_startend_val + String(tempBoilerSet, 1) + payloadvalue_startend_val +
                  ",\"" + OT + BOILER_CH_STATE + "\": " + payloadvalue_startend_val + String(status_CHActive ? payloadON : payloadOFF) + payloadvalue_startend_val +
                  ",\"" + OT + BOILER_SOFTWARE_CH_STATE_MODE + "\": \"" + String(boilermode) + "\",\"" +
                  OT + FLAME_STATE + "\": " + payloadvalue_startend_val + String(status_FlameOn ? payloadON : payloadOFF) + payloadvalue_startend_val +
                  ",\"" + OT + FLAME_LEVEL + "\": " + payloadvalue_startend_val + String(flame_level, 0) + payloadvalue_startend_val +
                  ",\"" + OT + FLAME_W + "\": " + payloadvalue_startend_val + String(flame_used_energy) + payloadvalue_startend_val +
                  ",\"" + OT + TEMP_CUTOFF + "\": " + payloadvalue_startend_val + String(cutOffTemp, 1) + payloadvalue_startend_val +
                  "}").c_str(), mqtt_Retain); //"heat" : "off")    boilermode.c_str(),1);// ? "auto" : "heat" : "off",1); //heatingEnabled ? "1" : "0",1);  //"heat" : "off",1);

  client.publish(DIAG_TOPIC.c_str(),
                 ("{\"" + OT + DIAGS_OTHERS_FAULT + "\": " + payloadvalue_startend_val + String(status_Fault ? payloadON : payloadOFF) + payloadvalue_startend_val +
                  ",\"" + OT + DIAGS_OTHERS_DIAG + "\": " + payloadvalue_startend_val + String(status_Diagnostic ? payloadON : payloadOFF) + payloadvalue_startend_val +
                  ",\"" + OT + INTEGRAL_ERROR_GET_TOPIC + "\": " + payloadvalue_startend_val + String(ierr) + payloadvalue_startend_val +
                  "}").c_str(), mqtt_Retain); //"heat" : "off")

  publishhomeassistantconfig++; // zwiekszamy licznik wykonan wyslania mqtt by co publishhomeassistantconfigdivider wysłań wysłać autoconfig discovery dla homeassisatnt
  if (publishhomeassistantconfig % publishhomeassistantconfigdivider == 0)
  {
    client.setBufferSize(2048);

    // homeassistant/sensor/BB050B_OPENTHERM_OT10_lo/config = {"name":"Opentherm OPENTHERM OT10 lo","stat_t":"tele/tasmota_BB050B/SENSOR","avty_t":"tele/tasmota_BB050B/LWT","pl_avail":"Online","pl_not_avail":"Offline","uniq_id":"BB050B_OPENTHERM_OT10_lo","dev":{"ids":["BB050B"]},"unit_of_meas":" ","ic":"mdi:eye","frc_upd":true,"val_tpl":"{{value_json['OPENTHERM']['OT10']['lo']}}"} (retained) problem
    // 21:16:02.724 MQT: homeassistant/sensor/BB050B_OPENTHERM_OT10_hi/config = {"name":"Opentherm OPENTHERM OT10 hi","stat_t":"tele/tasmota_BB050B/SENSOR","avty_t":"tele/tasmota_BB050B/LWT","pl_avail":"Online","pl_not_avail":"Offline","uniq_id":"BB050B_OPENTHERM_OT10_hi","dev":{"ids":["BB050B"]},"unit_of_meas":" ","ic":"mdi:eye","frc_upd":true,"val_tpl":"{{value_json['OPENTHERM']['OT10']['hi']}}"} (retained)
    client.publish((DIAG_HABS_TOPIC + "_" + DIAGS_OTHERS_FAULT + "/config").c_str(), ("{\"name\":\"" + OT + DIAGS_OTHERS_FAULT + "\",\"uniq_id\": \"" + OT + DIAGS_OTHERS_FAULT + "\",\"stat_t\":\"" + DIAG_TOPIC + "\",\"payload_on\": " + payloadON + ",\"payload_off\": " + payloadOFF + ",\"val_tpl\":\"{{value_json." + OT + DIAGS_OTHERS_FAULT + "}}\",\"frc_upd\":true,\"dev_cla\":\"problem\",\"unit_of_meas\": \" \",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    client.publish((DIAG_HABS_TOPIC + "_" + DIAGS_OTHERS_DIAG + "/config").c_str(), ("{\"name\":\"" + OT + DIAGS_OTHERS_DIAG + "\",\"uniq_id\": \"" + OT + DIAGS_OTHERS_DIAG + "\",\"stat_t\":\"" + DIAG_TOPIC + "\",\"payload_on\": " + payloadON + ",\"payload_off\": " + payloadOFF + ",\"val_tpl\":\"{{value_json." + OT + DIAGS_OTHERS_DIAG + "}}\",\"frc_upd\":true,\"unit_of_meas\": \" \",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    client.publish((DIAG_HA_TOPIC + "_" + INTEGRAL_ERROR_GET_TOPIC + "/config").c_str(), ("{\"name\":\"" + OT + INTEGRAL_ERROR_GET_TOPIC + "\",\"uniq_id\": \"" + OT + INTEGRAL_ERROR_GET_TOPIC + "\",\"stat_t\":\"" + DIAG_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + INTEGRAL_ERROR_GET_TOPIC + "}}\",\"frc_upd\":true,\"unit_of_meas\": \" \",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    client.publish((DIAG_HA_TOPIC + "_" + LOGS + "/config").c_str(), ("{\"name\":\"" + OT + LOGS + "\",\"uniq_id\": \"" + OT + LOGS + "\",\"stat_t\":\"" + LOG_GET_TOPIC + "\",\"val_tpl\":\"{{ value }}\",\"frc_upd\":true,\"unit_of_meas\": \" \",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);

    client.publish((ROOM_OTHERS_HA_TOPIC + "_" + ROOM_OTHERS_TEMPERATURE + "/config").c_str(), ("{\"name\":\"" + OT + ROOM_OTHERS_TEMPERATURE + "\",\"uniq_id\": \"" + OT + ROOM_OTHERS_TEMPERATURE + "\",\"stat_t\":\"" + ROOM_OTHERS_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + ROOM_OTHERS_TEMPERATURE + "}}\",\"frc_upd\":true,\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    client.publish((ROOM_OTHERS_HA_TOPIC + "_" + ROOM_OTHERS_TEMPERATURE_SETPOINT + "/config").c_str(), ("{\"name\":\"" + OT + ROOM_OTHERS_TEMPERATURE_SETPOINT + "\",\"uniq_id\": \"" + OT + ROOM_OTHERS_TEMPERATURE_SETPOINT + "\",\"stat_t\":\"" + ROOM_OTHERS_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + ROOM_OTHERS_TEMPERATURE_SETPOINT + "}}\",\"frc_upd\":true,\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    client.publish((ROOM_OTHERS_HA_TOPIC + "_" + ROOM_OTHERS_PRESSURE + "/config").c_str(), ("{\"name\":\"" + OT + ROOM_OTHERS_PRESSURE + "\",\"uniq_id\": \"" + OT + ROOM_OTHERS_PRESSURE + "\",\"stat_t\":\"" + ROOM_OTHERS_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + ROOM_OTHERS_PRESSURE + "}}\",\"frc_upd\":true,\"dev_cla\":\"pressure\",\"unit_of_meas\": \"hPa\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);

    client.publish((HOT_WATER_HA_TOPIC + "_" + HOT_WATER_TEMPERATURE + "/config").c_str(), ("{\"name\":\"" + OT + HOT_WATER_TEMPERATURE + "\",\"uniq_id\": \"" + OT + HOT_WATER_TEMPERATURE + "\",\"stat_t\":\"" + HOT_WATER_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + HOT_WATER_TEMPERATURE + "}}\",\"frc_upd\":true,\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    client.publish((HOT_WATER_HA_TOPIC + "_" + HOT_WATER_TEMPERATURE_SETPOINT + "/config").c_str(), ("{\"name\":\"" + OT + HOT_WATER_TEMPERATURE_SETPOINT + "\",\"uniq_id\": \"" + OT + HOT_WATER_TEMPERATURE_SETPOINT + "\",\"stat_t\":\"" + HOT_WATER_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + HOT_WATER_TEMPERATURE_SETPOINT + "}}\",\"frc_upd\":true,\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    client.publish((HOT_WATER_HABS_TOPIC + "_" + HOT_WATER_CH_STATE + "/config").c_str(), ("{\"name\":\"" + OT + HOT_WATER_CH_STATE + "\",\"uniq_id\": \"" + OT + HOT_WATER_CH_STATE + "\",\"stat_t\":\"" + HOT_WATER_TOPIC + "\",\"payload_on\": " + payloadON + ",\"payload_off\": " + payloadOFF + ",\"val_tpl\":\"{{value_json." + OT + HOT_WATER_CH_STATE + "}}\",\"frc_upd\":true,\"dev_cla\":\"heat\",\"unit_of_meas\":\" \",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    client.publish((HOT_WATER_HABS_TOPIC + "_" + HOT_WATER_SOFTWARE_CH_STATE + "/config").c_str(), ("{\"name\":\"" + OT + HOT_WATER_SOFTWARE_CH_STATE + "\",\"uniq_id\": \"" + OT + HOT_WATER_SOFTWARE_CH_STATE + "\",\"stat_t\":\"" + HOT_WATER_TOPIC + "\",\"payload_on\": " + payloadON + ",\"payload_off\": " + payloadOFF + ",\"val_tpl\":\"{{value_json." + OT + HOT_WATER_SOFTWARE_CH_STATE + "}}\",\"frc_upd\":true,\"dev_cla\":\"heat\",\"unit_of_meas\": \" \",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);

    client.publish((BOILER_HA_TOPIC + "_" + BOILER_TEMPERATURE + "/config").c_str(), ("{\"name\":\"" + OT + BOILER_TEMPERATURE + "\",\"uniq_id\": \"" + OT + BOILER_TEMPERATURE + "\",\"stat_t\":\"" + BOILER_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + BOILER_TEMPERATURE + "}}\",\"frc_upd\":true,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    client.publish((BOILER_HA_TOPIC + "_" + BOILER_TEMPERATURE_RET + "/config").c_str(), ("{\"name\":\"" + OT + BOILER_TEMPERATURE_RET + "\",\"uniq_id\": \"" + OT + BOILER_TEMPERATURE_RET + "\",\"stat_t\":\"" + BOILER_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + BOILER_TEMPERATURE_RET + "}}\",\"frc_upd\":true,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    client.publish((BOILER_HA_TOPIC + "_" + BOILER_TEMPERATURE_SETPOINT + "/config").c_str(), ("{\"name\":\"" + OT + BOILER_TEMPERATURE_SETPOINT + "\",\"uniq_id\": \"" + OT + BOILER_TEMPERATURE_SETPOINT + "\",\"stat_t\":\"" + BOILER_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + BOILER_TEMPERATURE_SETPOINT + "}}\",\"frc_upd\":true,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    client.publish((BOILER_HABS_TOPIC + "_" + BOILER_CH_STATE + "/config").c_str(), ("{\"name\":\"" + OT + BOILER_CH_STATE + "\",\"uniq_id\": \"" + OT + BOILER_CH_STATE + "\",\"stat_t\":\"" + BOILER_TOPIC + "\",\"payload_on\": " + payloadON + ",\"payload_off\": " + payloadOFF + ",\"val_tpl\":\"{{value_json." + OT + BOILER_CH_STATE + "}}\",\"frc_upd\":true,\"dev_cla\":\"heat\",\"unit_of_meas\": \" \",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    client.publish((BOILER_HA_TOPIC + "_" + BOILER_SOFTWARE_CH_STATE_MODE + "/config").c_str(), ("{\"name\":\"" + OT + BOILER_SOFTWARE_CH_STATE_MODE + "\",\"uniq_id\": \"" + OT + BOILER_SOFTWARE_CH_STATE_MODE + "\",\"stat_t\":\"" + BOILER_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + BOILER_SOFTWARE_CH_STATE_MODE + "}}\",\"frc_upd\":true,\"unit_of_meas\": \" \",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    client.publish((BOILER_HABS_TOPIC + "_" + FLAME_STATE + "/config").c_str(), ("{\"name\":\"" + OT + FLAME_STATE + "\",\"uniq_id\": \"" + OT + FLAME_STATE + "\",\"stat_t\":\"" + BOILER_TOPIC + "\",\"payload_on\": " + payloadON + ",\"payload_off\": " + payloadOFF + ",\"val_tpl\":\"{{value_json." + OT + FLAME_STATE + "}}\",\"frc_upd\":true,\"dev_cla\":\"heat\",\"unit_of_meas\":\" \",\"ic\": \"mdi:fire\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    client.publish((BOILER_HA_TOPIC + "_" + FLAME_LEVEL + "/config").c_str(), ("{\"name\":\"" + OT + FLAME_LEVEL + "\",\"uniq_id\": \"" + OT + FLAME_LEVEL + "\",\"stat_t\":\"" + BOILER_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + FLAME_LEVEL + "}}\",\"frc_upd\":true,\"dev_cla\":\"power\",\"unit_of_meas\":\"%\",\"ic\": \"mdi:fire\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    client.publish((BOILER_HA_TOPIC + "_" + FLAME_W + "/config").c_str(), ("{\"name\":\"" + OT + FLAME_W + "\",\"uniq_id\": \"" + OT + FLAME_W + "\",\"stat_t\":\"" + BOILER_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + FLAME_W + "}}\",\"frc_upd\":true,\"dev_cla\":\"power\",\"unit_of_meas\":\"W\",\"state_class\":\"measurement\",\"ic\": \"mdi:fire\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);

    client.publish((BOILER_HA_TOPIC + "_" + TEMP_CUTOFF + "/config").c_str(), ("{\"name\":\"" + OT + TEMP_CUTOFF + "\",\"uniq_id\": \"" + OT + TEMP_CUTOFF + "\",\"stat_t\":\"" + BOILER_TOPIC + "\",\"val_tpl\":\"{{value_json." + OT + TEMP_CUTOFF + "}}\",\"frc_upd\":true,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"ic\": \"mdi:thermometer\",\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);

    client.publish((HOT_WATER_HACLI_TOPIC + "_climate/config").c_str(), ("{\"name\":\"" + OT + "Hot Water\",\"uniq_id\": \"" + OT + "Hot_Water\", \
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
\"min_temp\": \"" + oplo + "\", \
\"max_temp\": \"" + ophi + "\", \
\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    client.publish((BOILER_HACLI_TOPIC + "_climate/config").c_str(), ("{\"name\":\"" + OT + "Boiler CO\",\"uniq_id\": \"" + OT + "Boiler_CO\", \
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
\"min_temp\": \"" + opcolo + "\", \
\"max_temp\": \"" + opcohi + "\", \
\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    client.publish((ROOM_OTHERS_HACLI_TOPIC + "_climate/config").c_str(), ("{\"name\":\"" + OT + "Boiler RoomTemp Control CO\",\"uniq_id\": \"" + OT + "Boiler_RoomTemp_Control_CO\", \
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
\"min_temp\": \"" + roomtemplo + "\", \
\"max_temp\": \"" + roomtemphi + "\", \
\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
    client.publish((ROOM_OTHERS_HACLI_TOPIC + "cutoff_climate/config").c_str(), ("{\"name\":\"" + OT + "CutoffTemp\",\"uniq_id\": \"" + OT + "CutoffTemp\", \
\"modes\":\"\", \
\"icon\": \"mdi:water-pump\", \
\"current_temperature_topic\":\"" + NEWS_GET_TOPIC + "\", \
\"current_temperature_template\":\"{{value}}\", \
\"temperature_command_topic\":\"" + TEMP_CUTOFF_SET_TOPIC + "\", \
\"temperature_state_topic\":\"" + BOILER_TOPIC + "\", \
\"temperature_state_template\":\"{{value_json." + OT + TEMP_CUTOFF + "}}\", \
\"temperature_unit\":\"C\", \
\"min_temp\": \"" + cutofflo + "\", \
\"max_temp\": \"" + cutoffhi + "\", \
\"qos\":" + QOS + "," + deviceid + "}").c_str(), mqtt_Retain);
  }

#ifdef debug
  Serial.print("Current temperature: " + String(roomtemp) + " °C ");
#endif
  WebSerial.print("Current temperature 18B20: " + String(roomtemp) + " °C ");
  String tempSource = (millis() - lastTempSet > extTempTimeout_ms)
                          ? "(internal sensor)"
                          : "(external sensor)";
#ifdef debug
  Serial.println(tempSource);
#endif
  WebSerial.println(tempSource);
}

String convertPayloadToStr(byte *payload, unsigned int length)
{
  char s[length + 1];
  s[length] = 0;
  for (int i = 0; i < length; ++i)
    s[i] = payload[i];
  String tempRequestStr(s);
  return tempRequestStr;
}

bool isValidNumber(String str)
{
  bool valid = true;
  for (byte i = 0; i < str.length(); i++)
  {
    char ch = str.charAt(i);
    valid &= isDigit(ch) ||
             ch == '+' || ch == '-' || ch == ',' || ch == '.' ||
             ch == '\r' || ch == '\n';
  }
  return valid;
}

void callback(char *topic, byte *payload, unsigned int length)
{
  const String topicStr(topic);

  String payloadStr = convertPayloadToStr(payload, length);
  payloadStr.trim();

  if (topicStr == TEMP_SETPOINT_SET_TOPIC)
  {
    payloadStr.replace(",", ".");
#ifdef debug
    Serial.println("Set target temperature: " + payloadStr);
#endif
    WebSerial.println("Set target temperature: " + payloadStr);
    float sp1 = payloadStr.toFloat();
    if (isnan(sp1) || !isValidNumber(payloadStr))
    {
#ifdef debug
      Serial.println("Setpoint is not a valid number, ignoring...");
#endif
      WebSerial.println("Setpoint is not a valid number, ignoring...");
    }
    else
    {
      if (sp1 > opcohi)
        sp1 = opcohi;
      if (sp1 < opcolo)
        sp1 = opcolo;
      //      sp = sp1;
      tempBoilerSet = sp1;
      op_override = sp1; // when no auto heating then this is temp to heat CO
      receivedmqttdata = true;
    }
  }
  else if (topicStr == ROOM_TEMP_SET_TOPIC)
  {
    payloadStr.replace(",", ".");
    float t1 = payloadStr.toFloat();
    if (isnan(t1) || !isValidNumber(payloadStr))
    {
#ifdef debug
      Serial.println(F("Current temp set is not a valid number, ignoring..."));
#endif
      WebSerial.println(F("Current temp set is not a valid number, ignoring..."));
    }
    else
    {
      if (t1 > roomtemphi)
        t1 = roomtemphi;
      if (t1 < roomtemplo)
        t1 = roomtemplo;
      sp = t1;
      // t = t1;
      receivedmqttdata = true;
      WebSerial.println("Room Temp set: " + payloadStr);
    }
  }
  else if (topicStr == MODE_SET_TOPIC)
  {
#ifdef debug
    Serial.println("Set mode: " + payloadStr);
#endif
    WebSerial.println("Set mode: " + payloadStr);
    if (payloadStr == "on" or payloadStr == "ON" or payloadStr == "On" or payloadStr == "1" or payloadStr == "heat" or payloadStr == "HEAT" or payloadStr == "Heat")
    {
      heatingEnabled = true;
      receivedmqttdata = true;
      automodeCO = false;
      tempBoilerSet = op_override;
      WebSerial.println("CO mode " + payloadStr);
    }
    else if (payloadStr == "off" or payloadStr == "OFF" or payloadStr == "Off" or payloadStr == "0")
    {
      heatingEnabled = false;
      receivedmqttdata = true;
      automodeCO = false;
      WebSerial.println("CO mode " + payloadStr);
    }
    else if (payloadStr == "auto" or payloadStr == "AUTO" or payloadStr == "Auto" or payloadStr == "2")
    {
      automodeCO = true;
      receivedmqttdata = true;
      WebSerial.println("CO mode " + payloadStr);
    }
    else
#ifdef debug
      Serial.println("Unknown mode " + payloadStr);
#endif
    WebSerial.println("Unknown mode " + payloadStr);
  }
  else if (topicStr == TEMP_DHW_SET_TOPIC)
  {
    payloadStr.replace(",", ".");
    float dhwTarget1 = payloadStr.toFloat();
    if (isnan(dhwTarget1) || !isValidNumber(payloadStr))
    {
#ifdef debug
      Serial.println(F("DHW target is not a valid number, ignoring..."));
#endif
      WebSerial.println(F("DHW target is not a valid number, ignoring..."));
    }
    else
    {
      if (dhwTarget1 > ophi)
        dhwTarget1 = ophi;
      if (dhwTarget1 < oplo)
        dhwTarget1 = oplo;
      dhwTarget = dhwTarget1;
      receivedmqttdata = true;
    }
  }
  else if (topicStr == STATE_DHW_SET_TOPIC)
  {
    if (payloadStr == "on" or payloadStr == "ON" or payloadStr == "On" or payloadStr == "1" or payloadStr == "heat" or payloadStr == "HEAT" or payloadStr == "Heat")
    {
      enableHotWater = true;
      receivedmqttdata = true;
    }
    else if (payloadStr == "off" or payloadStr == "OFF" or payloadStr == "Off" or payloadStr == "0")
    {
      enableHotWater = false;
      receivedmqttdata = true;
    }
    else
    {
#ifdef debug
      Serial.println("Unknown domestic hot water state " + payloadStr);
#endif
      WebSerial.println("Unknown domestic hot water state " + payloadStr);
    }
  }
  else if (topicStr == SETPOINT_OVERRIDE_SET_TOPIC)
  {
    payloadStr.replace(",", ".");
    float op_override1 = payloadStr.toFloat();
    if (isnan(op_override1) || !isValidNumber(payloadStr))
    {
#ifdef debug
      Serial.println(F("Setpoint override is not a valid number, ignoring..."));
#endif
      WebSerial.println(F("Setpoint override is not a valid number, ignoring..."));
    }
    else
    {
      op_override = op_override1;
      lastSpSet = millis();
      receivedmqttdata = true;
    }
  }
  else if (topicStr == NEWS_GET_TOPIC)
  {
    payloadStr.replace(",", ".");
    float op_news = payloadStr.toFloat();
    if (isnan(op_news) || !isValidNumber(payloadStr))
    {
#ifdef debug
      Serial.println(F("NEWS is not a valid number, ignoring..."));
#endif
      WebSerial.println(F("NEWS is not a valid number, ignoring..."));
    }
    else
    {
      temp_NEWS = op_news;
      lastNEWSSet = millis();
      temp_NEWS_count = 0;
      receivedmqttdata = true;

      WebSerial.print(F("NEWS updated from MQTT, to: "));
      WebSerial.println(temp_NEWS);
    }
  }
  else if (topicStr == TEMP_CUTOFF_SET_TOPIC)
  {
    payloadStr.replace(",", ".");
    float op_coff = payloadStr.toFloat();
    if (isnan(op_coff) || !isValidNumber(payloadStr))
    {
#ifdef debug
      Serial.println(F("Temp cutoff is not a valid number, ignoring..."));
#endif
      WebSerial.println(F("Temp cutoff is not a valid number, ignoring..."));
    }
    else
    {
      if (op_coff > cutoffhi)
        op_coff = cutoffhi;
      if (op_coff < cutofflo)
        op_coff = cutofflo;
      cutOffTemp = op_coff;
      lastcutOffTempSet = millis();
      receivedmqttdata = true;
    }
  }
  else if (topicStr == COPUMP_GET_TOPIC)
  {
    if (payloadStr == "on" or payloadStr == "ON" or payloadStr == "On" or payloadStr == "1")
    {
      CO_PumpWorking = true;
      automodeCO = true;
      receivedmqttdata = true;
    }
    else if (payloadStr == "off" or payloadStr == "OFF" or payloadStr == "Off" or payloadStr == "0")
    {
      CO_PumpWorking = false;
      receivedmqttdata = true;
    }
    else
    {
#ifdef debug
      Serial.println("Unknown copump Working state " + payloadStr);
#endif
      WebSerial.println("Unknown copump Working state " + payloadStr);
    }
  }
  else if (topicStr == SETPOINT_OVERRIDE_RESET_TOPIC)
  {
    lastSpSet = 0;
#ifdef debug
    Serial.println(F("Setpoint override reset"));
#endif
    WebSerial.println(F("Setpoint override reset"));
  }
  else
  {
#ifdef debug
    Serial.printf("Unknown topic: %s\r\n", topic);
#endif
    WebSerial.println("Unknown topic: " + String(topic));
    return;
  }

  lastUpdate = 0;
  //  saveConfig();
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected() and mqtt_offline_retrycount < mqtt_offline_retries)
  {
#ifdef debug
    Serial.print(F("Attempting MQTT connection..."));
#endif
    WebSerial.print(F("Attempting MQTT connection..."));
    const char *clientId = "opentherm-thermostat-test";

    if (client.connect(clientId, mqtt_user, mqtt_password))
    {
#ifdef debug
      Serial.println(F("ok"));
#endif
      WebSerial.println(F("ok"));
      mqtt_offline_retrycount = 0;

      client.subscribe(TEMP_SETPOINT_SET_TOPIC.c_str());
      client.subscribe(MODE_SET_TOPIC.c_str());
      client.subscribe(ROOM_TEMP_SET_TOPIC.c_str());
      client.subscribe(TEMP_DHW_SET_TOPIC.c_str());
      client.subscribe(STATE_DHW_SET_TOPIC.c_str());
      client.subscribe(SETPOINT_OVERRIDE_SET_TOPIC.c_str());
      client.subscribe(SETPOINT_OVERRIDE_RESET_TOPIC.c_str());

      client.subscribe(NEWS_GET_TOPIC.c_str());
      client.subscribe(COPUMP_GET_TOPIC.c_str());
      client.subscribe(TEMP_CUTOFF_SET_TOPIC.c_str());
    }
    else
    {
#ifdef debug
      Serial.print(F(" failed, rc="));
#endif
      WebSerial.print(F(" failed, rc="));
#ifdef debug
      Serial.print(client.state());
#endif
      WebSerial.print(client.state());
#ifdef debug
      Serial.println(F(" try again in 5 seconds"));
#endif
      WebSerial.println(F(" try again in 5 seconds"));
      mqtt_offline_retrycount++;
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  delay(3000);
  Serial.println(F("Starting... Delay3000..."));

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
    saveConfig(); // overwrite with the default settings
  }

  Serial.println(("Connecting to " + String(ssid)));
  WiFi.mode(WIFI_STA);
  WiFi.hostname(String(me_lokalizacja).c_str());
  WiFi.setAutoReconnect(true);
  WiFi.setAutoConnect(true);
  WiFi.persistent(true);
  WiFi.begin(ssid, pass);

  int deadCounter = 20;
  while (WiFi.status() != WL_CONNECTED && deadCounter-- > 0)
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(("Failed to connect to " + String(ssid)));
    while (true)
      ;
  }
  else
  {
    Serial.println(F("ok"));
  }


  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  ot.begin(handleInterrupt);

  // Init DS18B20 sensor
  sensors.begin();
  sensors.requestTemperatures();
  sensors.setWaitForConversion(false); // switch to async mode
  roomtemp, roomtemp_last = sensors.getTempCByIndex(0);
  ts = millis();
  lastTempSet = -extTempTimeout_ms;

  started = millis();

  WebSerial.begin(&webserver);
  WebSerial.msgCallback(recvMsg);
  WebServers();
}

void loop()
{
  unsigned long now = millis() + 100; // TO AVOID compare -2>10000 which is true ??? why?
  // check mqtt is available and connected in other case check values in api.
  if (status_FlameOn) {
    flame_used_power += ((flame_level/100)*boiler_rated_kWh/((millis()-start_flame_time)/hour)*1000);
    //flame_elapsed_time = millis()-start_flame_time;
  }

  if (mqtt_offline_retrycount == mqtt_offline_retries)
  {
    if ((now - lastmqtt_reconnect) > mqtt_offline_reconnect_after_ms)
    {
      lastmqtt_reconnect = now;
      mqtt_offline_retrycount = 0;
      WebSerial.println(F("MQTT connection problem -now reset retry counter and try again..."));
    }
    else
    {
      WebSerial.println(F("MQTT connection problem -now try get temp data alternative way (room temp and NEWS temp and Carbon CO Water pump status"));
      // best place to function get values from http when mqtt is unavailable
      //lastNEWSSet = now; // reset counter news temp with alternative parse value way
      //temp_NEWS_count = 0;
    }
  }
  else
  {
    if (!client.connected())
    {
      WebSerial.println(F("MQTT connection problem -try to connect again..."));
      reconnect();
    }
    else
    {
      client.loop();
    }
  }

  if ((now - lastUpdate) > statusUpdateInterval_ms)
  {
    lastUpdate = now;
    opentherm_update_data(lastUpdatemqtt); // According OpenTherm Specification from Ihnor Melryk Master requires max 1s interval communication -przy okazji wg czasu update mqtt zrobie odczyt dallas
  }
  if (((now - lastUpdatemqtt) > mqttUpdateInterval_ms) or (receivedmqttdata == true))
  {
    lastUpdatemqtt = now;
    updateData();
  }
  //#define abs(x) ((x)>0?(x):-(x))
  if ((now - lastNEWSSet) > temp_NEWS_interval_reduction_time_ms)
  { // at every 0,5hour lower temp NEWS when no communication why -2>1800000 is true ???
    WebSerial.print("now: ");
    WebSerial.println(String(now));
    WebSerial.print("lastNEWSSet: ");
    WebSerial.println(String(lastNEWSSet));
    WebSerial.print("temp_NEWS_interval_reduction_time_ms: ");
    WebSerial.println(String(temp_NEWS_interval_reduction_time_ms));
    lastNEWSSet = now;
    temp_NEWS_count++;
    if (temp_NEWS > cutOffTemp)
    {
      temp_NEWS = temp_NEWS - temp_NEWS * 0.05;
      WebSerial.println(F("Lowering by 5% temp_NEWS (no communication) -after 10times execute every 30minutes lowered temp NEWS"));
    }
    else
    {
      temp_NEWS = cutOffTemp;
    }
    if (temp_NEWS_count > 10)
    {
      CO_PumpWorking = false; // assume that we loose mqtt connection to other system where is co pump controlled -so after 10 times lowered NEWS temp by 5% we also disable CO_Pump_Working to allow heat by this heater -default it counts 5hours no communication
      WebSerial.println(F("Force disable CO_PumpWorking flag -after 10times execute every 30minutes lowered temp NEWS"));
      temp_NEWS_count = 0;
    }
    // dobre miejsce do try get data by http api
    WebSerial.println(F("Korekta temperatury NEWS z braku połaczenia -pomniejszona o 5%"));
  }

  // if WiFi is down, try reconnecting
  if ((WiFi.status() != WL_CONNECTED) && (now - WiFipreviousMillis >= WiFiinterval))
  {
    Serial.print(now);
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.begin(ssid, pass);
    WiFipreviousMillis = now;
  }

  if ((now - lastSaveConfig) > save_config_every)
  {
    lastSaveConfig = now;
    WebSerial.println(F("Saving config to EEPROM memory..."));
    saveConfig(); // According OpenTherm Specification from Ihnor Melryk Master requires max 1s interval communication -przy okazji wg czasu update mqtt zrobie odczyt dallas
  }

  static bool failFlag = false;
  bool fail = now - lastTempSet > extTempTimeout_ms && now - lastSpSet > spOverrideTimeout_ms + now;
  if (fail)
  {
    if (!failFlag)
    {
      failFlag = true;
#ifdef debug
      Serial.printf("Neither temperature nor setpoint provided, setting heating water to %.1f\r\n", noCommandSpOverride);
#endif
      WebSerial.println("Neither temperature nor setpoint provided, setting heating water to " + String(noCommandSpOverride));
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
