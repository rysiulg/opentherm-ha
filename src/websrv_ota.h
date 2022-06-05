String uptimedana(unsigned long started_local);
String processor(const String var);
bool isValidNumber(String str);

#define uptimelink "uptimelink"
#define dallThermometerS "dallThermometerS"
#define tempCOThermometerS "tempCOThermometerS"
#define tempCOThermometerSetS "tempCOThermometerSetS"
#define tempCORETThermometerS "tempCORETThermometerS"
#define tempCOset "tempCOset"
#define stopkawebsite "stopkawebsite"
#define tempHWThermometerS "tempHWThermometerS"
#define tempHWThermometerSetS "tempHWThermometerSetS"
#define tempHWset "tempHWset"
#define cutOffTempS "cutOffTempS"
#define cutOffTempSet "cutOffTempSet"
#define cutOffTempVAL "tempCutOffset"
#define flameprocS "flameprocS"
#define flameprocSVAL "flameprocSVAL"
#define NEWS_lastTimeS "NEWS_lastTimeS"
#define NEWS_lastTime "NEWS_lastTime"
#define do_stopkawebsiteS "do_stopkawebsiteS"

#define tempROOMThermometerSetS "tempROOMThermometerSetS"
#define tempROOMThermometerS "tempROOMThermometerS"
#define tempROOMset "tempROOMset"

const char* PARAM_MESSAGE_tempCOset = tempCOset;
const char* PARAM_MESSAGE_tempHWset = tempHWset;
const char* PARAM_MESSAGE_cutOffTempSet = cutOffTempVAL;
const char* PARAM_MESSAGE_tempROOMset = tempROOMset;

uint8_t mac[6] = {(uint8_t)strtol(WiFi.macAddress().substring(0,2).c_str(),0,16), (uint8_t)strtol(WiFi.macAddress().substring(3,5).c_str(),0,16),(uint8_t)strtol(WiFi.macAddress().substring(6,8).c_str(),0,16),(uint8_t)strtol(WiFi.macAddress().substring(9,11).c_str(),0,16),(uint8_t)strtol(WiFi.macAddress().substring(12,14).c_str(),0,16),(uint8_t)strtol(WiFi.macAddress().substring(15,17).c_str(),0,16)};
unsigned long  started = 0; //do mierzenia czasu uptime bez resetu

const char htmlup[] PROGMEM = R"rawliteral(
  <form method='POST' action='/doUpdate' enctype='multipart/form-data'><input type='file' name='update' accept=".bin,.bin.gz"><input type='submit' value='Update'></form>)rawliteral";
//static const char successResponse[] PROGMEM =  "<META http-equiv=\"refresh\" content=\"15;URL=/\">Update Success! Rebooting...";

String do_stopkawebsite();
//******************************************************************************************

String PrintHex8(const uint8_t *data, char separator, uint8_t length) // prints 8-bit data in hex , uint8_t length
{
  uint8_t lensep = sizeof(separator);
  int dod = 0;
  if (separator == 0x0) {
    lensep = 0;
    dod = 1;
  }
  wdt_reset();
  if (lensep > 1) dod = 1 - lensep;
  char tmp[length * (2 + lensep) + dod - lensep];
  byte first;
  byte second;
  for (int i = 0; (i + 0) < length; i++) {
    first = (data[i] >> 4) & 0x0f;
    second = data[i] & 0x0f;
    // base for converting single digit numbers to ASCII is 48
    // base for 10-16 to become lower-case characters a-f is 87
    // note: difference is 39
    tmp[i * (2 + lensep)] = first + 48;
    tmp[i * (2 + lensep) + 1] = second + 48;
    if ((i) < length and (i) + 1 != length) tmp[i * (2 + lensep) + 2] = separator;
    if (first > 9) tmp[i * (2 + lensep)] += 39;
    if (second > 9) tmp[i * (2 + lensep) + 1] += 39;

  }
  tmp[length * (2 + lensep) + 0 - lensep] = 0;
#ifdef debug
  Serial.print(F("MAC Addr: "));
  Serial.println(tmp);
#endif
  //     debugA("%s",tmp);
  return tmp;
}

//*********************************************************************************************

//******************************************************************************************
//     background-color: #01DF3A;
//
//  <div class='s'><svg version='1.1' width="75px" height="75px" id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
  <meta http-equiv="refresh" content="3600">
  <title>%me_lokalizacja%</title>

  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">

  <style>
    %stylesectionadd%
  </style>
</head>
<body>


  <h2>%ver%</h2>
  <p>
  <sup class="units">Uptime <b><span id="%uptime%">%uptimewart%</span></B></sup>
  <br/>
<sup class="units">%dane%</sup>
  </p>
   %bodywstaw%
  <p>
    <span id="%do_stopkawebsiteS%" class="units">%stopkawebsite%</span>
    %stopkawebsite0%
  </p>
</body>
<script>
  %scriptsectionreplace%
</script>
</html>)rawliteral";
//******************************************************************************************






void handleGetTemp() {
	//digitalWrite(BUILTIN_LED, 1);
	//webserver.send(200, "text/plain", String(123));
	//digitalWrite(BUILTIN_LED, 0);
}

//#include <Update.h>
size_t content_len;
void printProgress(size_t prg, size_t sz) {
  Serial.printf("Progress: %d%%\n", (prg*100)/content_len);
  WebSerial.println("Progress: "+String((prg*100)/content_len));
}

void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  //#define UPDATE_SIZE_UNKNOWN 0XFFFFFFFF
  //uint32_t free_space = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
  if (!index){
    Serial.println(F("Update"));
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
    response->addHeader("Refresh", "15");
    response->addHeader("Location", "/");
    request->send(response);
    content_len = request->contentLength();
    // if filename includes spiffs, update the spiffs partition
    int cmd = U_FLASH; //(filename.indexOf("spiffs") > -1) ? U_SPIFFS : U_FLASH;
    Update.runAsync(true);
    if (!Update.begin(content_len, cmd)){
      Update.printError(Serial);
    }
  }
  Serial.println(F("Write data..."));
  if (Update.write(data, len) != len) {
    Update.printError(Serial);
  }
  if (final) {
    if (!Update.end(true)){
      Update.printError(Serial);
    } else {
      Serial.println("Update complete");
      Serial.flush();
      AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
      response->addHeader("Refresh", "20");
      response->addHeader("Location", "/");
      request->send(response);
      delay(100);
      ESP.restart();
    }
  }
}


void WebServers() {
  #ifdef debug
    Serial.println(F("subWerbServers..."));
  #endif
  webserver.on("/update", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/html", htmlup);
    }).setAuthentication("", "");
  webserver.on("/doUpdate", HTTP_POST,
      [&](AsyncWebServerRequest *request) {
      // the request handler is triggered after the upload has finished...
      // create the response, add header, and send response
      AsyncWebServerResponse *response = request->beginResponse((Update.hasError())?500:200, "text/plain", (Update.hasError())?"FAIL":"OK");
      response->addHeader("Connection", "close");
      response->addHeader("Access-Control-Allow-Origin", "*");
      request->send(response);
      },
    [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data,
                  size_t len, bool final) {handleDoUpdate(request, filename, index, data, len, final);}
  ).setAuthentication("", "");;


  // [](AsyncWebServerRequest * request) {},
  // [](AsyncWebServerRequest * request, const String & filename, size_t index, uint8_t *data,
  //    size_t len, bool final) {
  //   handleDoUpdate(request, filename, index, data, len, final);

  // }).setAuthentication("", "");
  webserver.onNotFound([](AsyncWebServerRequest * request) {
    request->send(404);
  });
  webserver.on("/" , HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html; charset=utf-8",  index_html, processor);
  }).setAuthentication("", "");
    // Send a GET request to <IP>/get?message=<message>
  webserver.on("/" uptimelink , HTTP_GET, [](AsyncWebServerRequest * request) {
  //  request.setAuthentication("", "");
    request->send(200, "text/plain; charset=utf-8", String(uptimedana(started)));
  }).setAuthentication("", "");
  webserver.on("/" dallThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(temp_NEWS));
  }).setAuthentication("", "");
  webserver.on("/" tempCOThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(tempBoiler,1));
  }).setAuthentication("", "");
  webserver.on("/" tempCOThermometerSetS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(tempBoilerSet,1));
  }).setAuthentication("", "");
  webserver.on("/" tempCORETThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(retTemp,1));
  }).setAuthentication("", "");
  webserver.on("/" tempHWThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(tempCWU,1));
  }).setAuthentication("", "");
  webserver.on("/" tempHWThermometerSetS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(dhwTarget,1));
  }).setAuthentication("", "");
  webserver.on("/" cutOffTempS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(cutOffTemp,1));
  }).setAuthentication("", "");
  webserver.on("/" cutOffTempS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(uptimedana(lastNEWSSet)));
  }).setAuthentication("", "");
  webserver.on("/" do_stopkawebsiteS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", do_stopkawebsite());
  }).setAuthentication("", "");




  webserver.on("/" tempROOMThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(roomtemp,1));
  }).setAuthentication("", "");
  webserver.on("/" tempROOMThermometerSetS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(sp,1));
  }).setAuthentication("", "");
  webserver.on("/" NEWS_lastTimeS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(uptimedana(lastNEWSSet)));
  }).setAuthentication("", "");



  webserver.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String message;
      if (request->hasParam(PARAM_MESSAGE_tempCOset)) {
        message = request->getParam(PARAM_MESSAGE_tempCOset)->value();
        String ident = String(millis())+": WebReceived change Boiler CO Set ";
        if (PayloadtoValidFloatCheck(message))  //wrong value are displayed in function
            {
              #ifdef debug
              Serial.print(ident);
              #endif
              WebSerial.print(ident);
              tempBoilerSet = PayloadtoValidFloat(message, true, roomtemplo, roomtemphi);
              op_override = tempBoilerSet; // when no auto heating then this is temp to heat CO
              receivedmqttdata = true;
            } else {
              #ifdef debug
              Serial.println(ident + " is not a valid number, ignoring...");
              #endif
              WebSerial.println(ident + " is not a valid number, ignoring...");
            }
      } else {
        //message = "No message sent tempCOset";
      }
      if (request->hasParam(PARAM_MESSAGE_cutOffTempSet)) {
          message = request->getParam(PARAM_MESSAGE_cutOffTempSet)->value();
        String ident = String(millis())+": WebReceived change TempCutOff Set ";
        if (PayloadtoValidFloatCheck(message))  //wrong value are displayed in function
            {
              #ifdef debug
              Serial.print(ident);
              #endif
              WebSerial.print(ident);
              cutOffTemp = PayloadtoValidFloat(message, true, cutofflo, cutoffhi);
              lastcutOffTempSet = millis();
              receivedmqttdata = true;
            } else {
              #ifdef debug
              Serial.println(ident + " is not a valid number, ignoring...");
              #endif
              WebSerial.println(ident + " is not a valid number, ignoring...");
            }
      } else {
        //message = "No message sent PARAM_MESSAGE_cutOffTempSet";
      }
      if (request->hasParam(PARAM_MESSAGE_tempHWset)) {
        message = request->getParam(PARAM_MESSAGE_tempHWset)->value();
        String ident = String(millis())+": WebReceived DHW target ";
        if (PayloadtoValidFloatCheck(message))  //wrong value are displayed in function
            {
              #ifdef debug
              Serial.print(ident);
              #endif
              WebSerial.print(ident);
              dhwTarget = PayloadtoValidFloat(message, true, oplo, ophi);
              receivedmqttdata = true;
            } else {
              #ifdef debug
              Serial.println(ident + " is not a valid number, ignoring...");
              #endif
              WebSerial.println(ident + " is not a valid number, ignoring...");
            }
      } else {
        //message = "No message sent PARAM_MESSAGE_tempHWset";
      }
      if (request->hasParam(PARAM_MESSAGE_tempROOMset)) {
          message = request->getParam(PARAM_MESSAGE_tempROOMset)->value();
        String ident = String(millis())+": WebReceived Room Target sp ";
        if (PayloadtoValidFloatCheck(message))  //wrong value are displayed in function
            {
              #ifdef debug
              Serial.print(ident);
              #endif
              WebSerial.print(ident);
              sp = PayloadtoValidFloat(message, true, roomtemplo, roomtemphi);
              receivedmqttdata = true;
            } else {
              #ifdef debug
              Serial.println(ident + " is not a valid number, ignoring...");
              #endif
              WebSerial.println(ident + " is not a valid number, ignoring...");
            }
      } else {
        //message = "No message sent PARAM_MESSAGE_tempROOMset";
      }
      if (request->hasParam("boilermodewww")) { //tempCOset
        message = request->getParam("boilermodewww")->value();
        String ident = String(millis())+": WebReceived Modde Set ";
        #ifdef debug
        Serial.print(ident + "Set mode: ");
        #endif
        WebSerial.print(ident + "Set mode: ");
        if (PayloadStatus(message, true))
        {
          heatingEnabled = true;
          automodeCO = false;
          tempBoilerSet = op_override;
          receivedmqttdata = true;
          WebSerial.println("CO mode " + message);
        }
        else if (PayloadStatus(message, false))
        {
          heatingEnabled = false;
          automodeCO = false;
          receivedmqttdata = true;
          WebSerial.println("CO mode " + message);
        }
        else if (message == "AUTO" or message == "2")
        {
          automodeCO = true;
          receivedmqttdata = true;
          WebSerial.println("CO mode " + message);
        } else {
          #ifdef debug
          Serial.println("Unknown mode " + message);
          #endif
          WebSerial.println("Unknown mode " + message);
        }
        WebSerial.print(F("WebReceived change Boiler Mode CO to: "));
        WebSerial.print(String(message)+" ");
        WebSerial.println(automodeCO ? "Auto" : "Heat/Off" );
      } else {
        //message = "No message sent tempCOset";
      }
      if (request->hasParam("boilerhwwww")) { //tempCOset
          message = request->getParam("boilerhwwww")->value();
          String ident = String(millis())+": DHW State enableHotWater ";
          #ifdef debug
          Serial.print(ident);
          #endif
          WebSerial.print(ident);
          receivedmqttdata = true;
          if (PayloadStatus(message, true)) enableHotWater = true;
          else if (PayloadStatus(message, false)) enableHotWater = false;
          else
          {
            receivedmqttdata = false;
            #ifdef debug
            Serial.println("Unknown: "+String(message));
            #endif
            WebSerial.println("Unknown: "+String(message));
          }
          if (receivedmqttdata) {
            #ifdef debug
            Serial.println(enableHotWater ? "heat" : "off" );
            #endif
            WebSerial.println(enableHotWater ? "heat" : "off" );
          }
      } else {
        //message = "No message sent tempCOset";
      }
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Update value");
    response->addHeader("Refresh", "1");
    response->addHeader("Location", "/");
    request->send(response);
//      request->send(200, "text/plain", "<HEAD> <meta http-equiv=\"refresh\" content=\"0;url=/\"> </head>");
//    saveConfig();
  });
    // Send a POST request to <IP>/post with a form field message set to <message>
/*     webserver.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
        String message;
        if (request->hasParam(PARAM_MESSAGE, true)) {
            message = request->getParam(PARAM_MESSAGE, true)->value();
        } else {
            message = "No message sent";
        }
        request->send(200, "text/plain", "Hello, POST: " + message);
    });
 */
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Server",me_lokalizacja);
  DefaultHeaders::Instance().addHeader("Title",me_lokalizacja);

	webserver.begin();
  #ifdef debug
	Serial.println("HTTP server started");
  #endif
  Update.onProgress(printProgress);

  // Add service to MDNS-SD
  //MDNS.addService("http", "tcp", 80);
}


String processor(const String var) {


  #ifdef debug
//    Serial.print(F("Start processor: "));
//    Serial.println(var);
  #endif
  if (var == "ver") {
    String a = F("ESP CO Server dla: ")+String(me_lokalizacja);
    a += F("<br>v. ") + String(me_version);
    a += F("<br><font size=\"2\" color=\"DarkGreen\">");
    a += client.connected()? "MQTT "+String(msg_Connected)+": "+String(mqtt_server)+":"+String(mqtt_port) : "MQTT "+String(msg_disConnected)+": "+String(mqtt_server)+":"+String(mqtt_port) ;  //1 conn, 0 not conn
    #ifdef ENABLE_INFLUX
    a += F(" + INFLUXDB: ")+String(INFLUXDB_DB_NAME)+F("/")+String(InfluxMeasurments);
    a += F("</font>");
    #endif
    return a;
  }

  if (var == "dane") {
    String a = F("MAC: <B>");
    #ifdef debug
      Serial.println(F("Raport Hosta "));
    #endif
    a += PrintHex8(mac, ':', sizeof(mac) / sizeof(mac[0]));
    a += F("</b>&nbsp;&nbsp;<B>");
    a += F("</B>WiFi (RSSI): <B>");
    a += WiFi.RSSI();
    a += F("dBm</b> CRT:");
    a += String(runNumber);
    a += F("<br>");
    a += LastboilerResponseError;
    a += F("<br></B>");

    #ifdef debug
      Serial.println(F("Raport Hosta po read_eprom: "));
    #endif
    return String(a).c_str();
  }

  if (var == "uptimewart") {
    return String(uptimedana(started));
  }
  if (var == "me_lokalizacja") {
    return String(me_lokalizacja);
  }

  if (var == "uptime") {
    return String(uptimelink);
  }
  if (var == "do_stopkawebsiteS") return String(do_stopkawebsiteS);

  if (var=="stylesectionadd") {
    String ptr;
    ptr=F("html{font-family:Arial;display:inline-block;margin:0 auto;text-align:center}\
    h2{font-size:2.1rem}\
    p{font-size:2.2rem}\
    .units{font-size:1.1rem}\
    .dht-labels{font-size:1.3rem;vertical-align:middle;padding-bottom:8px}\
    .dht-labels-temp{font-size:3.3rem;font-weight:700;vertical-align:middle;padding-bottom:8px}\
    table,td,th{border-color:green;border-collapse:collapse;border-style:outset;margin-left:auto;margin-right:auto;border:0;text-align:center;padding-left: 5px;padding-right: 10px;padding-top: 5px;padding-bottom: 10px;}\
    input{margin:.4rem}\
    td{height:auto;width:auto}");
    ptr+=F("body{background-color:lightcyan;}");
    return String(ptr).c_str();
  }

  if (var == "stopkawebsite") {
    return do_stopkawebsite();
  }
  if (var == "stopkawebsite0") {
    String ptr;
      ptr = F("<br><span class='units'><a href='/update' target=\"_blank\">")+String(Update_web_link)+F("</a> &nbsp; &nbsp;&nbsp; <a href='/webserial' target=\"_blank\">")+String(Web_Serial)+F("</a>&nbsp;");
      ptr += F("<br>&copy; ");
      ptr += stopka;
      ptr += F("<br>");
    return String(ptr).c_str();
  }


  if (var=="bodywstaw") {
    String ptr;
    const float tempstep=0.5;
    const String tempicon=F("<i class=\"fas fa-thermometer-half\" style=\"color:#059e8a;font-size:28px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;");
    ptr=F("<form action=\"/get\">");

    ptr+=F("<p>")+tempicon+F("<span class=\"dht-labels\">")+String(Temp_NEWS)+F("</span><B><span class=\"dht-labels-temp\" id=\"")+String(dallThermometerS)+F("\">")+String(temp_NEWS)+F("</span><sup class=\"units\">&deg;C</sup></B>");
    ptr+=F("<font size=\"4\" color=\"blue\">")+String(ActualFrom)+F("<B><span id=\"")+String(NEWS_lastTimeS)+F("\">")+String(uptimedana(lastNEWSSet))+F("</span></B> </font></p>");

    ptr+=F("<p><table><tr>");
    ptr+=F("<td><B><LABEL FOR=\"BOILMOD\">")+String(Boler_mode)+F("</LABEL></B><br><INPUT TYPE=\"Radio\" ID=\"BOILMOD\" Name=\"boilermodewww\" Value=\"2\" ")+String(automodeCO?"Checked":"")+F(">")+String(Automatic_mode)+F("</td>");
    ptr+=F("<td><B><LABEL FOR=\"HWMOD\">")+String(DHW_Mode)+F("</LABEL></B><br><INPUT TYPE=\"Radio\" ID=\"HWMOD\" Name=\"boilerhwwww\" Value=\"1\" ")+String(enableHotWater?"Checked":"")+F(">")+String(Heating)+F("</td>");
    ptr+=F("</tr><tr><td><INPUT TYPE=\"Radio\" ID=\"BOILMOD\" Name=\"boilermodewww\" Value=\"1\" ")+String(automodeCO?"":"Checked")+F(">")+String(Heating)+F("/")+String(Off)+F("</td>");
    ptr+=F("<td><INPUT TYPE=\"Radio\" ID=\"HWMOD\" Name=\"boilerhwwww\" Value=\"0\" ")+String(enableHotWater?"":"Checked")+F(">")+String(Off)+F("</td></tr>");

    ptr+=F("<tr><td>");
    ptr+=tempicon+F("<span class=\"dht-labels\">")+String(DHW_Temp)+F("</span>");
    ptr+=F("<br><B>");
    ptr+=F("<span class=\"dht-labels-temp\" id=\"")+String(tempHWThermometerS)+F("\">")+String(tempCWU,1)+F("</span><sup class=\"units\">&deg;C</sup></B>");
    ptr+=F("<br></td><td>");
    ptr+=tempicon+F("<span class=\"dht-labels\">")+String(DHW_Temp_Set)+F("</span>");
    ptr+=F("<br>");
    ptr+=F("<font size=\"4\" color=\"red\"><input type=\"number\" id=\"T")+String(tempHWThermometerSetS)+F("\" min=\"")+String(oplo,1)+F("\" max=\"")+String(ophi,1)+F("\" step=\"")+String(tempstep,1)+F("\" value=\"")+String(dhwTarget,1)+F("\" style=\"width:auto\" onchange=\"uTI(this.value, '")+String(tempHWThermometerSetS)+F("');\"><sup class=\"units\">&deg;C</sup></B><input id=\"")+String(tempHWThermometerSetS)+F("\" type=\"range\" min=\"")+String(oplo,1)+F("\" max=\"")+String(ophi,1)+F("\" step=\"")+String(tempstep,1)+F("\" name=\"")+String(PARAM_MESSAGE_tempHWset)+F("\" value=\"")+String(dhwTarget,1)+F("\" style=\"width:50px\" onchange=\"uTI(this.value, 'T")+String(tempHWThermometerSetS)+F("');\">");
    ptr+=F("<input type=\"submit\" style=\"width:45px\"></font>");
    ptr+=F("</td></tr>");

    ptr+=F("<tr><td>");
    ptr+=tempicon+F("<span class=\"dht-labels\">")+String(Boiler_And_CO_Temperature)+F("</span>");
    ptr+=F("<br><B>");
    ptr+=F("<span class=\"dht-labels-temp\" id=\"")+String(tempCOThermometerS)+F("\">")+String(tempBoiler,1)+F("</span><sup class=\"units\">&deg;C</sup></B>");
    ptr+=F("<br></td><td>");
    ptr+=tempicon+F("<span class=\"dht-labels\">")+String(Set_Temperature_for_CO_heat)+F("</span>");
    ptr+=F("<br>");
    ptr+=F("<font size=\"4\" color=\"red\"><input type=\"number\" id=\"T")+String(tempCOThermometerSetS)+F("\" min=\"")+String(opcolo,1)+F("\" max=\"")+String(opcohi,1)+F("\" step=\"")+String(tempstep,1)+F("\" value=\"")+String(tempBoilerSet,1)+F("\" style=\"width:auto\" onchange=\"uTI(this.value, '")+String(tempCOThermometerSetS)+F("');\"><sup class=\"units\">&deg;C</sup></B><input id=\"")+String(tempCOThermometerSetS)+F("\" type=\"range\" min=\"")+String(opcolo,1)+F("\" max=\"")+String(opcohi,1)+F("\" step=\"")+String(tempstep,1)+F("\" name=\"")+String(PARAM_MESSAGE_tempCOset)+F("\" value=\"")+String(tempBoilerSet,1)+F("\" style=\"width:50px\" onchange=\"uTI(this.value, 'T")+String(tempCOThermometerSetS)+F("');\">");
    ptr+=F("<input type=\"submit\" style=\"width:45px\"></font>");
    ptr+=F("</td></tr>");

   ptr+=F("<tr><td>");
    ptr+=tempicon+F("<span class=\"dht-labels\">")+String(Room_Temp)+F("</span>");
    ptr+=F("<br><B>");
    ptr+=F("<span class=\"dht-labels-temp\" id=\"")+String(tempROOMThermometerS)+F("\">")+String(roomtemp,1)+F("</span><sup class=\"units\">&deg;C</sup></B>");
    ptr+=F("<br></td><td>");
    ptr+=tempicon+F("<span class=\"dht-labels\">")+String(Room_Temp_Set)+F("</span>");
    ptr+=F("<br>");
    ptr+=F("<font size=\"4\" color=\"red\"><input type=\"number\" id=\"T")+String(tempROOMThermometerSetS)+F("\" min=\"")+String(roomtemplo,1)+F("\" max=\"")+String(roomtemphi,1)+F("\" step=\"")+String(tempstep,1)+F("\" value=\"")+String(sp,1)+F("\" style=\"width:auto\" onchange=\"uTI(this.value, '")+String(tempROOMThermometerSetS)+F("');\"><sup class=\"units\">&deg;C</sup></B><input id=\"")+String(tempROOMThermometerSetS)+F("\" type=\"range\" min=\"")+String(roomtemplo,1)+F("\" max=\"")+String(roomtemphi,1)+F("\" step=\"")+String(tempstep,1)+F("\" name=\"")+String(PARAM_MESSAGE_tempROOMset)+F("\" value=\"")+String(sp,1)+F("\" style=\"width:50px\" onchange=\"uTI(this.value, 'T")+String(tempROOMThermometerSetS)+F("');\">");
    ptr+=F("<input type=\"submit\" style=\"width:45px\"></font>");
    ptr+=F("</td></tr>");

    ptr+=F("<tr><td>");
    ptr+=tempicon+F("<span class=\"dht-labels\"><font color=\"")+String((retTemp<boiler_50_30_ret)? "darkgreen" : "black")+F("\">")+String(Return_temp)+F("</span>");
    ptr+=F("<br><B>");
    ptr+=F("<span class=\"dht-labels-temp\" id=\"")+String(tempCORETThermometerS)+F("\">")+String(retTemp,1)+F("</span><sup class=\"units\">&deg;C</sup></B></font>");
    ptr+=F("<br></td><td>");
    ptr+=tempicon+F("<span class=\"dht-labels\">")+String(Outside_Cutoff_Below)+F("</span>");
    ptr+=F("<br>");
    ptr+=F("<font size=\"4\" color=\"red\"><input type=\"number\" id=\"T")+String(cutOffTempS)+"\" min=\""+String(cutofflo,1)+"\" max=\""+String(cutoffhi,1)+"\" step=\""+String(tempstep,1)+F("\" value=\"")+String(cutOffTemp,1)+F("\" style=\"width:auto\" onchange=\"uTI(this.value, '")+String(cutOffTempS)+F("');\"><sup class=\"units\">&deg;C</sup></B><input id=\"")+String(cutOffTempS)+F("\" type=\"range\" min=\"")+String(cutofflo,1)+F("\" max=\"")+String(cutoffhi,1)+F("\" step=\"")+String(tempstep,1)+F("\" name=\"")+String(PARAM_MESSAGE_cutOffTempSet)+F("\" value=\"")+String(cutOffTemp,1)+F("\" style=\"width:50px\" onchange=\"uTI(this.value, 'T")+String(cutOffTempS)+F("');\">");
    ptr+=F("<input type=\"submit\" style=\"width:45px\"></font>");
    ptr+=F("</td></tr>");

    ptr+=F("</table></form><br>");

    return ptr.c_str();
  }


  if (var=="scriptsectionreplace") {
    String ptr;
    String tmp;
    unsigned long int step=125;
    unsigned long int refreshtime = 9100;
    const String function0 = "setInterval(function(){var e=new XMLHttpRequest;e.onreadystatechange=function(){4==this.readyState&&200==this.status&&(document.getElementById(\"";
    const String function1 = "\").innerHTML=this.responseText)},e.open(\"GET\",\"/";
    const String function2 = "\",!0),e.send()},";
    const String function3 = ");\n";

    ptr=F("function uTI(e,n){document.getElementById(n).value=e};\n");
    tmp=String(uptimelink);  //spanid
    refreshtime+=step; ptr+=function0+tmp+function1+tmp+function2+String(refreshtime/2)+");\n";
    tmp=String(tempCOThermometerS); refreshtime+=step; ptr+=function0+tmp+function1+tmp+function2+String(refreshtime)+");\n";
    tmp=String(tempCOThermometerSetS); refreshtime+=step; ptr+=function0+tmp+function1+tmp+function2+String(refreshtime)+");\n";
    tmp=""+String(tempCOThermometerSetS); refreshtime+=step; ptr+=function0+tmp+function1+tmp+function2+String(refreshtime)+");\n";
    tmp=String(dallThermometerS); refreshtime+=step; ptr+=function0+F("T")+tmp+function1+tmp+function2+String(refreshtime)+");\n";
    tmp=String(tempCORETThermometerS); refreshtime+=step; ptr+=function0+tmp+function1+tmp+function2+String(refreshtime)+");\n";
    tmp=String(tempHWThermometerS); refreshtime+=step; ptr+=function0+tmp+function1+tmp+function2+String(refreshtime)+");\n";
    tmp=String(tempHWThermometerSetS); refreshtime+=step; ptr+=function0+tmp+function1+tmp+function2+String(refreshtime)+");\n";
    tmp=""+String(tempHWThermometerSetS); refreshtime+=step; ptr+=function0+F("T")+tmp+function1+tmp+function2+String(refreshtime)+");\n";
    tmp=String(NEWS_lastTimeS); refreshtime+=step; ptr+=function0+tmp+function1+tmp+function2+String(refreshtime)+");\n";
    tmp=String(tempROOMThermometerS); refreshtime+=step; ptr+=function0+tmp+function1+tmp+function2+String(refreshtime)+");\n";
    tmp=String(tempROOMThermometerSetS); refreshtime+=step; ptr+=function0+tmp+function1+tmp+function2+String(refreshtime)+");\n";
    tmp=""+String(tempROOMThermometerSetS); refreshtime+=step; ptr+=function0+F("T")+tmp+function1+tmp+function2+String(refreshtime)+");\n";
    tmp=String(cutOffTempS); refreshtime+=step; ptr+=function0+tmp+function1+tmp+function2+String(refreshtime)+");\n";
    tmp=""+String(cutOffTempS); refreshtime+=step; ptr+=function0+F("T")+tmp+function1+tmp+function2+String(refreshtime)+");\n";
    tmp=String(do_stopkawebsiteS); refreshtime+=step; ptr+=function0+tmp+function1+tmp+function2+String(refreshtime)+");\n";
    return String(ptr).c_str();
  }
  #ifdef debug
//    Serial.print(F("End processor "));
//    Serial.println(var);
  #endif
  return String();

}

String do_stopkawebsite() {
      String ptr;
      ptr = "&nbsp;";
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
      ptr += "<br>"+String(Flame_total)+"<B>"+String(flame_used_power_kwh,4)+"kWh</B>";
    return String(ptr);
}
//******************************************************************************************
String uptimedana(unsigned long started_local) {
  String wynik = " ";
  unsigned long  partia = millis() - started_local;
  if (partia<1000) return "< 1 "+String(t_sek)+" ";
  #ifdef debug
    Serial.print(F("Uptimedana: "));
  #endif
  if (partia >= 24 * 60 * 60 * 1000 ) {
    unsigned long  podsuma = partia / (24 * 60 * 60 * 1000);
    partia -= podsuma * 24 * 60 * 60 * 1000;
    wynik += (String)podsuma + " "+String(t_day)+" ";

  }
  if (partia >= 60 * 60 * 1000 ) {
    unsigned long  podsuma = partia / (60 * 60 * 1000);
    partia -= podsuma * 60 * 60 * 1000;
    wynik += (String)podsuma + " "+String(t_hour)+" ";
  }
  if (partia >= 60 * 1000 ) {
    unsigned long  podsuma = partia / (60 * 1000);
    partia -= podsuma * 60 * 1000;
    wynik += (String)podsuma + " "+String(t_min)+" ";
    //Serial.println(podsuma);
  }
  if (partia >= 1 * 1000 ) {
    unsigned long  podsuma = partia / 1000;
    partia -= podsuma * 1000;
    wynik += (String)podsuma + " "+String(t_sek)+" ";
    //Serial.println(podsuma);
  }
  #ifdef debug
    Serial.println(wynik);
  #endif
  //wynik += (String)partia + "/1000";  //pomijam to wartosci <1sek
  return wynik;
}

#include <EEPROM.h>

#define CONFIG_VERSION "V01" sensitive_sizeS

// Where in EEPROM?
#define CONFIG_START 32

typedef struct
{
  char version[6]; // place to detect if settings actually are written
  bool heatingEnabled;
  bool enableHotWater;
  bool automodeCO;
  float tempBoilerSet;   //temp boiler set -mainly used in auto mode and for display on www actual temp
  float sp;              //romtempsetpoint
  float cutOffTemp;
  float op_override;     //boiler tempset on heat mode
  float dhwTarget;       //hot water temp set
  float roomtemp;        //now is static sensor so for while save last value
  float temp_NEWS;
  char ssid[sensitive_size];
  char pass[sensitive_size];
  char mqtt_server[sensitive_size*2];
  char mqtt_user[sensitive_size];
  char mqtt_password[sensitive_size];
  int mqtt_port;
  char COPUMP_GET_TOPIC[255];  //temperatura outside avg NEWS
  char NEWS_GET_TOPIC[255];   //pompa CO status
  char NEWS_GET_TOPIC1[255];   //pompa CO status for 1st temp room sensor
  char NEWS_GET_TOPIC2[255];   //pompa CO status for 2nd temp room sensor
} configuration_type;

// with DEFAULT values!
configuration_type CONFIGURATION;
configuration_type CONFTMP;

// load whats in EEPROM in to the local CONFIGURATION if it is a valid setting
bool loadConfig() {
  // is it correct?
  if (sizeof(CONFIGURATION)<1024) EEPROM.begin(1024); else EEPROM.begin(sizeof(CONFIGURATION)+128); //Size can be anywhere between 4 and 4096 bytes.
  EEPROM.get(1,runNumber);
  if (isnan(runNumber)) runNumber=0;
  runNumber++;
  EEPROM.get(1+sizeof(runNumber),flame_used_power_kwh);
  if (isnan(flame_used_power_kwh)) flame_used_power_kwh = 0;
  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2] &&
      EEPROM.read(CONFIG_START + 3) == CONFIG_VERSION[3] &&
      EEPROM.read(CONFIG_START + 4) == CONFIG_VERSION[4]){

  // load (overwrite) the local configuration struct
    for (unsigned int i=0; i<sizeof(configuration_type); i++){
      *((char*)&CONFIGURATION + i) = EEPROM.read(CONFIG_START + i);
    }
    heatingEnabled = CONFIGURATION.heatingEnabled;
    enableHotWater = CONFIGURATION.enableHotWater;
    automodeCO = CONFIGURATION.automodeCO;
    tempBoilerSet = CONFIGURATION.tempBoilerSet;
    sp = CONFIGURATION.sp;
    cutOffTemp = CONFIGURATION.cutOffTemp;
    op_override = CONFIGURATION.op_override;
    dhwTarget = CONFIGURATION.dhwTarget;
    roomtemp = CONFIGURATION.roomtemp;
    temp_NEWS = CONFIGURATION.temp_NEWS;
    strcpy(ssid, CONFIGURATION.ssid);
    strcpy(pass, CONFIGURATION.pass);
    strcpy(mqtt_server, CONFIGURATION.mqtt_server);
    strcpy(mqtt_user, CONFIGURATION.mqtt_user);
    strcpy(mqtt_password, CONFIGURATION.mqtt_password);
    mqtt_port = CONFIGURATION.mqtt_port;
    COPUMP_GET_TOPIC=String(CONFIGURATION.COPUMP_GET_TOPIC);
    NEWS_GET_TOPIC=String(CONFIGURATION.NEWS_GET_TOPIC);
    NEWS_GET_TOPIC=String(CONFIGURATION.NEWS_GET_TOPIC1);
    NEWS_GET_TOPIC=String(CONFIGURATION.NEWS_GET_TOPIC2);

    return true; // return 1 if config loaded
  }
  //try get only my important values

  return false; // return 0 if config NOT loaded
}

// save the CONFIGURATION in to EEPROM
void saveConfig() {
  #ifdef debug1
  Serial.println("Saving config...........................prepare ");
  #endif
  #ifdef enableWebSerial
  if (!starting) {WebSerial.println("Saving config...........................prepare ");}
  #endif
  double runtmp = 0;
  EEPROM.get(1,runtmp);
  if (runtmp != flame_used_power_kwh) {EEPROM.put(1+sizeof(runNumber), flame_used_power_kwh);}
  unsigned int temp =0;
  //firs read content of eeprom
  EEPROM.get(1,temp);
  //firs read content of eeprom
  EEPROM.get(1,temp);
  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2] &&
      EEPROM.read(CONFIG_START + 3) == CONFIG_VERSION[3] &&
      EEPROM.read(CONFIG_START + 4) == CONFIG_VERSION[4]){

  // load (overwrite) the local configuration struct
    for (unsigned int i=0; i<sizeof(configuration_type); i++){
      *((char*)&CONFTMP + i) = EEPROM.read(CONFIG_START + i);
    }
  }
//now compare and if changed than save
  if (temp != runNumber ||
      CONFTMP.heatingEnabled != heatingEnabled ||
      CONFTMP.enableHotWater != enableHotWater ||
      CONFTMP.automodeCO != automodeCO ||
      CONFTMP.tempBoilerSet != tempBoilerSet ||
      CONFTMP.sp != sp ||
      CONFTMP.cutOffTemp != cutOffTemp ||
      CONFTMP.op_override != op_override ||
      CONFTMP.dhwTarget != dhwTarget ||
      CONFTMP.roomtemp != roomtemp ||
      CONFTMP.temp_NEWS != temp_NEWS ||
      strcmp(CONFTMP.ssid,ssid) != 0 ||
      strcmp(CONFTMP.pass,pass) != 0 ||
      strcmp(CONFTMP.mqtt_server,mqtt_server) != 0 ||
      strcmp(CONFTMP.mqtt_user,mqtt_user) != 0 ||
      strcmp(CONFTMP.mqtt_password,mqtt_password) != 0 ||
      CONFTMP.mqtt_port != mqtt_port ||
      strcmp(CONFTMP.COPUMP_GET_TOPIC,COPUMP_GET_TOPIC.c_str()) != 0 ||
      strcmp(CONFTMP.NEWS_GET_TOPIC,NEWS_GET_TOPIC.c_str()) != 0 ||
      strcmp(CONFTMP.NEWS_GET_TOPIC1,NEWS_GET_TOPIC.c_str()) != 0 ||
      strcmp(CONFTMP.NEWS_GET_TOPIC2,NEWS_GET_TOPIC.c_str()) != 0 ) {
        EEPROM.put(1, runNumber);
        #ifdef debug1
        Serial.println(String(millis())+": Saving config........................... to EEPROM some data changed");
        #endif
        #ifdef enableWebSerial
        if (!starting) {WebSerial.println(String(millis())+": Saving config........................... to EEPROM some data changed");}
        #endif
        strcpy(CONFIGURATION.version,CONFIG_VERSION);
        CONFIGURATION.heatingEnabled = heatingEnabled;
        CONFIGURATION.enableHotWater = enableHotWater;
        CONFIGURATION.automodeCO = automodeCO;
        CONFIGURATION.tempBoilerSet = tempBoilerSet;
        CONFIGURATION.sp = sp;
        CONFIGURATION.cutOffTemp = cutOffTemp;
        CONFIGURATION.op_override = op_override;
        CONFIGURATION.dhwTarget = dhwTarget;
        CONFIGURATION.roomtemp = roomtemp;
        CONFIGURATION.temp_NEWS = temp_NEWS;
        strcpy(CONFIGURATION.ssid,ssid);
        strcpy(CONFIGURATION.pass,pass);
        strcpy(CONFIGURATION.mqtt_server,mqtt_server);
        strcpy(CONFIGURATION.mqtt_user,mqtt_user);
        strcpy(CONFIGURATION.mqtt_password,mqtt_password);
        CONFIGURATION.mqtt_port = mqtt_port;
        strcpy(CONFIGURATION.COPUMP_GET_TOPIC,COPUMP_GET_TOPIC.c_str());
        strcpy(CONFIGURATION.NEWS_GET_TOPIC,NEWS_GET_TOPIC.c_str());
        strcpy(CONFIGURATION.NEWS_GET_TOPIC1,NEWS_GET_TOPIC.c_str());
        strcpy(CONFIGURATION.NEWS_GET_TOPIC2,NEWS_GET_TOPIC.c_str());

        for (unsigned int i=0; i<sizeof(configuration_type); i++)
            {EEPROM.write(CONFIG_START + i, *((char*)&CONFIGURATION + i));}
        EEPROM.commit();
      }
}

void restart()
{
  delay(1500);
  WiFi.forceSleepBegin();
  webserver.end();
  WiFi.disconnect();
//      wifi.disconnect();
  delay(5000);
  WiFi.forceSleepBegin(); wdt_reset(); ESP.restart(); while (1)ESP.restart(); wdt_reset(); ESP.restart();
}

String getJsonVal(String json, String tofind)
{ //function to get value from json payload
  json.trim();
  tofind.trim();
  #ifdef debugweb
  WebSerial.println("json0: "+json);
  #endif
  if (!json.isEmpty() and !tofind.isEmpty() and json.startsWith("{") and json.endsWith("}"))  //check is starts and ends as json data and nmqttident null
  {
    json=json.substring(1,json.length()-1);                             //cut start and end brackets json
    #ifdef debugweb
    WebSerial.println("json1: "+json);
    #endif
    int tee=0; //for safety ;)
    #define maxtee 500
    while (tee!=maxtee)
    {         //parse all nodes
      int pos = json.indexOf(",",1);                //position to end of node:value
      if (pos==-1) {tee=maxtee;}
      String part;
      if (pos>-1) {part = json.substring(0,pos);} else {part=json; }       //extract parameter node:value
      part.replace("\"","");                      //clean from text indent
      part.replace("'","");
      json=json.substring(pos+1);                      //cut input for extracted node:value
      if (part.indexOf(":",1)==-1) {
        #ifdef debugweb
        WebSerial.println("Return no : data");
        #endif
        break;
      }
      String node=part.substring(0,part.indexOf(":",1));    //get node name
      node.trim();
      String nvalue=part.substring(part.indexOf(":",1)+1); //get node value
      nvalue.trim();
      #ifdef debugweb
      WebSerial.println("jsonx: "+json);
      WebSerial.println("tee: "+String(tee)+" tofind: "+tofind+" part: "+part+" node: "+node +" nvalue: "+nvalue + " indexof , :"+String(json.indexOf(",",1)));
      #endif
      if (tofind==node)
      {
         #ifdef debugweb
         WebSerial.println("Found node return val");
         #endif
        return nvalue;
        break;
      }
      tee++;
      #ifdef debugweb
      delay(1000);
      #endif
      if (tee>maxtee) {
        #ifdef debugweb
         WebSerial.println("tee exit: "+String(tee));
        #endif
        break;  //safety bufor
      }
    }
    #ifdef enableWebSerial
    WebSerial.println(String(millis())+": Json "+json+"  No mqttident contain searched value of "+tofind);
    #endif
  } else
  {
    #ifdef enableWebSerial
    WebSerial.println(String(millis())+": Inproper Json format or null: "+json+" to find: "+tofind);
    #endif
  }
  return "\0";
}