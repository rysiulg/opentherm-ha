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


#define tempROOMThermometerSetS "tempROOMThermometerSetS"
#define tempROOMThermometerS "tempROOMThermometerS"
#define tempROOMset "tempROOMset"

const char* PARAM_MESSAGE_tempCOset = tempCOset;
const char* PARAM_MESSAGE_tempHWset = tempHWset;
const char* PARAM_MESSAGE_cutOffTempSet = cutOffTempVAL;
const char* PARAM_MESSAGE_tempROOMset = tempROOMset;

uint8_t mac[6] = {strtol(WiFi.macAddress().substring(0,2).c_str(),0,16), strtol(WiFi.macAddress().substring(3,5).c_str(),0,16),strtol(WiFi.macAddress().substring(6,8).c_str(),0,16),strtol(WiFi.macAddress().substring(9,11).c_str(),0,16),strtol(WiFi.macAddress().substring(12,14).c_str(),0,16),strtol(WiFi.macAddress().substring(15,17).c_str(),0,16)};
unsigned long  started = 0; //do mierzenia czasu uptime bez resetu

const char htmlup[] PROGMEM = R"rawliteral(
  <form method='POST' action='/doUpdate' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>)rawliteral";

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
  <div class='s'><svg version='1.1' width="75px" height="75px" id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>
  <h2>%ver%</h2>
  <p>
  <sup class="units">Uptime <b><span id="%uptime%">%uptimewart%</span></B></sup>
  <br/>
<sup class="units">%dane%</sup>
  </p>
   %bodywstaw%
  <p>
    <span class="units">%stopkawebsite%</span>
  </p>
</body>
<script>
  %scriptsectionreplace%
</script>
</html>)rawliteral";
//******************************************************************************************



void handleUpdate(AsyncWebServerRequest *request) {
  request->send(200, "text/html", htmlup);
}


void handleGetTemp() {
	digitalWrite(BUILTIN_LED, 1);
	//webserver.send(200, "text/plain", String(123));
	digitalWrite(BUILTIN_LED, 0);
}

void printProgress(size_t prg, size_t sz) {
  int content_len = 255;
  #ifdef debug1
  Serial.printf("Progress: %d%%\n", (prg * 100) / content_len);
  #endif
  WebSerial.print(F("Progress: "));
  WebSerial.println((prg * 100) / sz);
}

void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  int content_len = 255;
//#define U_FLASH   0  //copied from ESP8266 library dla ESP32 sprawdzic
#define U_LittleFS  100
//#define U_AUTH    200
  if (!index) {
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
    response->addHeader("Refresh", "15");
    response->addHeader("Location", "/");
    request->send(response);
    #ifdef debug
    Serial.println(F("Update"));
    #endif
    content_len = request->contentLength();
    // if filename includes LittleFS, update the LittleFS partition
    int cmd = (filename.indexOf("LittleFS") > -1) ? U_LittleFS : U_FLASH;
#ifdef ESP8266
    Update.runAsync(true);
    if (!Update.begin(content_len, cmd)) {
#else
    if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) {
#endif
      Update.printError(Serial);
    }
  }

  if (Update.write(data, len) != len) {
    Update.printError(Serial);
//#ifdef ESP8266
  } else {
    printProgress(Update.progress(),Update.size());
   // Serial.printf("Progress: %d%%\n", (Update.progress() * 100) / Update.size());

//      WebSerial.print("Progressxx: ");
 //     WebSerial.println((Update.progress() * 100) / Update.size());
//#endif
  }

  if (final) {


    if (!Update.end(true)) {
      Update.printError(Serial);
    } else {
//      SaveEnergy();
      #ifdef debug
      Serial.println(F("Update complete"));
      #endif
      WebSerial.println(F("Update complete"));
      Serial.flush();
      WiFi.forceSleepBegin();
      webserver.end();
      WiFi.disconnect();
//      wifi.disconnect();
      delay(5000);
      WiFi.forceSleepBegin(); wdt_reset(); ESP.restart(); while (1)ESP.restart(); wdt_reset(); ESP.restart();
    }
  }
}


void WebServers() {
  #ifdef debug
    Serial.println(F("subWerbServers..."));
  #endif
  webserver.on("/update", HTTP_GET, [](AsyncWebServerRequest * request) {
//    AsyncWebHandler->setAuthentication("", "");
    handleUpdate(request);
  }).setAuthentication("", "");
  webserver.on("/doUpdate", HTTP_POST, [](AsyncWebServerRequest * request) {},
  [](AsyncWebServerRequest * request, const String & filename, size_t index, uint8_t *data,
     size_t len, bool final) {
    handleDoUpdate(request, filename, index, data, len, final);
  }).setAuthentication("", "");
  webserver.onNotFound([](AsyncWebServerRequest * request) {
    request->send(404);
  });
  webserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html; charset=utf-8",  index_html, processor);
  }).setAuthentication("", "");
    // Send a GET request to <IP>/get?message=<message>
  webserver.on("/"uptimelink , HTTP_GET, [](AsyncWebServerRequest * request) {
  //  request.setAuthentication("", "");
    request->send(200, "text/plain; charset=utf-8", String(uptimedana(started)));
  }).setAuthentication("", "");
  webserver.on("/"dallThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(temp_NEWS));
  }).setAuthentication("", "");
  webserver.on("/"tempCOThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(tempBoiler,1));
  }).setAuthentication("", "");
  webserver.on("/"tempCOThermometerSetS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(tempBoilerSet,1));
  }).setAuthentication("", "");
  webserver.on("/"tempCORETThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(retTemp,1));
  }).setAuthentication("", "");
  webserver.on("/"tempHWThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(tempCWU,1));
  }).setAuthentication("", "");
  webserver.on("/"tempHWThermometerSetS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(dhwTarget,1));
  }).setAuthentication("", "");
  webserver.on("/"cutOffTempS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(cutOffTemp,1));
  }).setAuthentication("", "");
  webserver.on("/"cutOffTempS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(uptimedana(temp_NEWS_count*temp_NEWS_interval_reduction_time_ms+lastNEWSSet)));
  }).setAuthentication("", "");

  webserver.on("/"tempROOMThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(roomtemp,1));
  }).setAuthentication("", "");
  webserver.on("/"tempROOMThermometerSetS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(sp,1));
  }).setAuthentication("", "");
  webserver.on("/"NEWS_lastTimeS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(uptimedana(temp_NEWS_count*temp_NEWS_interval_reduction_time_ms+lastNEWSSet)));
  }).setAuthentication("", "");



  webserver.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String message;
      if (request->hasParam(PARAM_MESSAGE_tempCOset)) { //tempCOset
          message = request->getParam(PARAM_MESSAGE_tempCOset)->value();
          message.trim();
          message.replace(",",".");
          float liczba = message.toFloat();
          if (isnan(liczba) || !isValidNumber(message)) {
            #ifdef debug
            Serial.println(F("Liczba not a valid number, ignoring..."));
            #endif
            WebSerial.println(F("Liczba not a valid number, ignoring..."));
          }
          else {
            if (liczba>opcohi) liczba=opcohi;
            if (liczba<opcolo) liczba=opcolo;
            tempBoilerSet = liczba;
            op_override = liczba;
 //           op = liczba;
            message = String(liczba);
            WebSerial.print(F("WebReceived change Boiler CO Set to: "));
            WebSerial.println(message);
          }
      } else {
        //message = "No message sent tempCOset"; tempHWset
      }
      if (request->hasParam(PARAM_MESSAGE_cutOffTempSet)) { //tempCOset
          message = request->getParam(PARAM_MESSAGE_cutOffTempSet)->value();
          message.trim();
          message.replace(",",".");
          float liczba = message.toFloat();
          if (isnan(liczba) || !isValidNumber(message)) {
            #ifdef debug
            Serial.println(F("Liczba not a valid number, ignoring..."));
            #endif
            WebSerial.println(F("Liczba not a valid number, ignoring..."));
          }
          else {
            if (liczba>cutoffhi) liczba=cutoffhi;
            if (liczba<cutofflo) liczba=cutofflo;
            cutOffTemp = liczba;
            message = String(liczba);
            WebSerial.print(F("WebReceived change TempCutOff Set to: "));
            WebSerial.println(message);
          }
      } else {
        //message = "No message sent tempCOset";
      }
      if (request->hasParam(PARAM_MESSAGE_tempHWset)) { //tempCOset
          message = request->getParam(PARAM_MESSAGE_tempHWset)->value();
          message.trim();
          message.replace(",",".");
          float liczba = message.toFloat();
          if (isnan(liczba) || !isValidNumber(message)) {
            #ifdef debug
            Serial.println(F("Liczba not a valid number, ignoring..."));
            #endif
            WebSerial.println(F("Liczba not a valid number, ignoring..."));
          }
          else {
            if (liczba>ophi) liczba=ophi;
            if (liczba<oplo) liczba=oplo;
            dhwTarget = liczba;
            message = String(liczba);
            WebSerial.print(F("WebReceived change Boiler CO Set to: "));
            WebSerial.println(message);
          }
      } else {
        //message = "No message sent tempCOset";
      }
      if (request->hasParam(PARAM_MESSAGE_tempROOMset)) { //tempCOset
          message = request->getParam(PARAM_MESSAGE_tempROOMset)->value();
          message.trim();
          message.replace(",",".");
          float liczba = message.toFloat();
          if (isnan(liczba) || !isValidNumber(message)) {
            #ifdef debug
            Serial.println(F("Liczba not a valid number, ignoring..."));
            #endif
            WebSerial.println(F("Liczba not a valid number, ignoring..."));
          }
          else {
            if (liczba>roomtemphi) liczba=roomtemphi;
            if (liczba<roomtemplo) liczba=roomtemplo;
            sp = liczba;
            message = String(liczba);
            WebSerial.print(F("WebReceived change Boiler CO Set to: "));
            WebSerial.println(message);
          }
      } else {
        //message = "No message sent tempCOset";
      }
      if (request->hasParam("boilermodewww")) { //tempCOset
          message = request->getParam("boilermodewww")->value();
          message.trim();
          message.replace(",",".");
          float liczba = message.toFloat();
          if (isnan(liczba) || !isValidNumber(message)) {
            #ifdef debug
            Serial.println(F("Liczba not a valid number, ignoring..."));
            #endif
            WebSerial.println(F("Liczba not a valid number, ignoring..."));
          }
          else {
            if (liczba==2) automodeCO=true;
            if (liczba==1) automodeCO=false;  //mode heat and off is controlled by outside temp cutoff
            message = String(liczba);
            WebSerial.print(F("WebReceived change Boiler Mode CO to: "));
            WebSerial.print(String(message)+" ");
            WebSerial.println(automodeCO ? "Auto" : "Heat/Off" );
          }
      } else {
        //message = "No message sent tempCOset";
      }
      if (request->hasParam("boilerhwwww")) { //tempCOset
          message = request->getParam("boilerhwwww")->value();
          message.trim();
          message.replace(",",".");
          float liczba = message.toFloat();
          if (isnan(liczba) || !isValidNumber(message)) {
            #ifdef debug
            Serial.println("Liczba not a valid number, ignoring...");
            #endif
            WebSerial.println("Liczba not a valid number, ignoring...");
          }
          else {
            if (liczba==1) enableHotWater=true;
            if (liczba==0) enableHotWater=false;
            message = String(liczba);
            WebSerial.print("WebReceived change HotWoter Mode: ");
            WebSerial.print(String(message)+" ");
            WebSerial.println(enableHotWater?"heat":"off");
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
}


String processor(const String var) {


  #ifdef debug
//    Serial.print(F("Start processor: "));
//    Serial.println(var);
  #endif
  if (var == "ver") {
    String a = "ESP CO Server v. ";
    a += me_version;
    a += "<br>";
    a += client.connected()? "MQTT "+String(msg_Connected)+": "+String(mqtt_server)+":"+String(mqtt_port) : "MQTT "+String(msg_disConnected)+": "+String(mqtt_server)+":"+String(mqtt_port) ;  //1 conn, 0 not conn
    return a;
  }

  if (var == "dane") {
    String a = "Raport dla Hosta: <B>";  //PrintHex8c(GUID,0x0,sizeof(GUID)/sizeof(GUID[0]));
    a += me_lokalizacja;
    a += "</B>&nbsp;&nbsp;&nbsp;MAC: <B>";
    #ifdef debug
      Serial.println(F("Raport Hosta "));
    #endif
    a += PrintHex8(mac, ':', sizeof(mac) / sizeof(mac[0]));
    a += "</b>&nbsp;&nbsp;<B>";
    a += "</B>WiFi (RSSI): <B>";
    a += WiFi.RSSI();
    a += "dBm</b> CRT:";
    a += String(runNumber);
    a += "<br>";
    a += LastboilerResponseError;
    a += "<br></B>";

    #ifdef debug
      Serial.println(F("Raport Hosta po read_eprom: "));
    #endif
    return String(a);
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

  if (var=="stylesectionadd") {
    String ptr;
    ptr="html{font-family:Arial;display:inline-block;margin:0 auto;text-align:center}\
    h2{font-size:2.1rem}\
    p{font-size:2.2rem}\
    .units{font-size:1.1rem}\
    .dht-labels{font-size:1.3rem;vertical-align:middle;padding-bottom:8px}\
    .dht-labels-temp{font-size:3.3rem;font-weight:700;vertical-align:middle;padding-bottom:8px}\
    table,td,th{border-color:green;border-collapse:collapse;border-style:outset;margin-left:auto;margin-right:auto;border:0;text-align:center;padding-left: 40px;padding-right: 50px;padding-top: 5px;padding-bottom: 10px;}\
    input{margin:.4rem}\
    td{height:auto;width:auto}";
    return String(ptr);
  }

  if (var == "stopkawebsite") {
    String ptr;
      ptr = "</span>&nbsp;";
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
      ptr += "<br><span class='units'><a href='/update'>"+String(Update_web_link)+"</a> &nbsp; &nbsp;&nbsp; <a href='/webserial'>"+String(Web_Serial)+"</a>&nbsp;";
      ptr += "<br>&copy; ";
      ptr += stopka;
      ptr += "<br>";
    return String(ptr);
  }

  if (var=="bodywstaw") {
    String ptr;
    const float tempstep=0.5;
    const String tempicon="<i class=\"fas fa-thermometer-half\" style=\"color:#059e8a;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;";
    ptr="<form action=\"/get\">";

    ptr+="<p>"+tempicon+"<span class=\"dht-labels\">"+String(Temp_NEWS)+"</span><B><span class=\"dht-labels-temp\" id=\""+String(dallThermometerS)+"\">"+String(temp_NEWS)+"</span><sup class=\"units\">&deg;C</sup></B>";
    ptr+="<font size=\"4\" color=\"blue\">"+String(ActualFrom)+"<B><span id=\""+String(NEWS_lastTimeS)+"\">"+String(uptimedana(temp_NEWS_count*temp_NEWS_interval_reduction_time_ms+lastNEWSSet))+"</span></B> </font></p>";

    ptr+="<p><table><tr>";
    ptr+="<td><B><LABEL FOR=\"BOILMOD\">"+String(Boler_mode)+"</LABEL></B><br><INPUT TYPE=\"Radio\" ID=\"BOILMOD\" Name=\"boilermodewww\" Value=\"2\" "+String(automodeCO?"Checked":"")+">"+String(Automatic_mode)+"</td>";
    ptr+="<td><B><LABEL FOR=\"HWMOD\">"+String(DHW_Mode)+"</LABEL></B><br><INPUT TYPE=\"Radio\" ID=\"HWMOD\" Name=\"boilerhwwww\" Value=\"1\" "+String(enableHotWater?"Checked":"")+">"+String(Heating)+"</td>";
    ptr+="</tr><tr><td><INPUT TYPE=\"Radio\" ID=\"BOILMOD\" Name=\"boilermodewww\" Value=\"1\" "+String(automodeCO?"":"Checked")+">"+String(Heating)+"/"+String(Off)+"</td>";
    ptr+="<td><INPUT TYPE=\"Radio\" ID=\"HWMOD\" Name=\"boilerhwwww\" Value=\"0\" "+String(enableHotWater?"":"Checked")+">"+String(Off)+"</td></tr>";

    ptr+="<tr><td>";
    ptr+=tempicon+"<span class=\"dht-labels\">"+String(DHW_Temp)+"</span>";
    ptr+="<br><B>";
    ptr+="<span class=\"dht-labels-temp\" id=\""+String(tempHWThermometerS)+"\">"+String(tempCWU,1)+"</span><sup class=\"units\">&deg;C</sup></B>";
    ptr+="<br></td><td>";
    ptr+=tempicon+"<span class=\"dht-labels\">"+String(DHW_Temp_Set)+"</span>";
    ptr+="<br>";
    ptr+="<font size=\"4\" color=\"red\"><input type=\"number\" id=\"T"+String(tempHWThermometerSetS)+"\" min=\""+String(oplo,1)+"\" max=\""+String(ophi,1)+"\" step=\""+String(tempstep,1)+"\" value=\""+String(dhwTarget,1)+"\" style=\"width:auto\" onchange=\"uTI(this.value, '"+String(tempHWThermometerSetS)+"');\"><sup class=\"units\">&deg;C</sup></B><input id=\""+String(tempHWThermometerSetS)+"\" type=\"range\" min=\""+String(oplo,1)+"\" max=\""+String(ophi,1)+"\" step=\""+String(tempstep,1)+"\" name=\""+String(PARAM_MESSAGE_tempHWset)+"\" value=\""+String(dhwTarget,1)+"\" style=\"width:50px\" onchange=\"uTI(this.value, 'T"+String(tempHWThermometerSetS)+"');\">";
    ptr+="<input type=\"submit\" style=\"width:45px\"></font>";
    ptr+="</td></tr>";

    ptr+="<tr><td>";
    ptr+=tempicon+"<span class=\"dht-labels\">"+String(Boiler_And_CO_Temperature)+"</span>";
    ptr+="<br><B>";
    ptr+="<span class=\"dht-labels-temp\" id=\""+String(tempCOThermometerS)+"\">"+String(tempBoiler,1)+"</span><sup class=\"units\">&deg;C</sup></B>";
    ptr+="<br></td><td>";
    ptr+=tempicon+"<span class=\"dht-labels\">"+String(Set_Temperature_for_CO_heat)+"</span>";
    ptr+="<br>";
    ptr+="<font size=\"4\" color=\"red\"><input type=\"number\" id=\"T"+String(tempCOThermometerSetS)+"\" min=\""+String(opcolo,1)+"\" max=\""+String(opcohi,1)+"\" step=\""+String(tempstep,1)+"\" value=\""+String(tempBoilerSet,1)+"\" style=\"width:auto\" onchange=\"uTI(this.value, '"+String(tempCOThermometerSetS)+"');\"><sup class=\"units\">&deg;C</sup></B><input id=\""+String(tempCOThermometerSetS)+"\" type=\"range\" min=\""+String(opcolo,1)+"\" max=\""+String(opcohi,1)+"\" step=\""+String(tempstep,1)+"\" name=\""+String(PARAM_MESSAGE_tempCOset)+"\" value=\""+String(tempBoilerSet,1)+"\" style=\"width:50px\" onchange=\"uTI(this.value, 'T"+String(tempCOThermometerSetS)+"');\">";
    ptr+="<input type=\"submit\" style=\"width:45px\"></font>";
    ptr+="</td></tr>";

   ptr+="<tr><td>";
    ptr+=tempicon+"<span class=\"dht-labels\">"+String(Room_Temp)+"</span>";
    ptr+="<br><B>";
    ptr+="<span class=\"dht-labels-temp\" id=\""+String(tempROOMThermometerS)+"\">"+String(roomtemp,1)+"</span><sup class=\"units\">&deg;C</sup></B>";
    ptr+="<br></td><td>";
    ptr+=tempicon+"<span class=\"dht-labels\">"+String(Room_Temp_Set)+"</span>";
    ptr+="<br>";
    ptr+="<font size=\"4\" color=\"red\"><input type=\"number\" id=\"T"+String(tempROOMThermometerSetS)+"\" min=\""+String(roomtemplo,1)+"\" max=\""+String(roomtemphi,1)+"\" step=\""+String(tempstep,1)+"\" value=\""+String(sp,1)+"\" style=\"width:auto\" onchange=\"uTI(this.value, '"+String(tempROOMThermometerSetS)+"');\"><sup class=\"units\">&deg;C</sup></B><input id=\""+String(tempROOMThermometerSetS)+"\" type=\"range\" min=\""+String(roomtemplo,1)+"\" max=\""+String(roomtemphi,1)+"\" step=\""+String(tempstep,1)+"\" name=\""+String(PARAM_MESSAGE_tempROOMset)+"\" value=\""+String(sp,1)+"\" style=\"width:50px\" onchange=\"uTI(this.value, 'T"+String(tempROOMThermometerSetS)+"');\">";
    ptr+="<input type=\"submit\" style=\"width:45px\"></font>";
    ptr+="</td></tr>";

   ptr+="<tr><td>";
    ptr+=tempicon+"<span class=\"dht-labels\">"+String(Return_temp)+"</span>";
    ptr+="<br><B>";
    ptr+="<span class=\"dht-labels-temp\" id=\""+String(tempCORETThermometerS)+"\">"+String(retTemp,1)+"</span><sup class=\"units\">&deg;C</sup></B>";
    ptr+="<br></td><td>";
    ptr+=tempicon+"<span class=\"dht-labels\">"+String(Outside_Cutoff_Below)+"</span>";
    ptr+="<br>";
    ptr+="<font size=\"4\" color=\"red\"><input type=\"number\" id=\"T"+String(cutOffTempS)+"\" min=\""+String(cutofflo,1)+"\" max=\""+String(cutoffhi,1)+"\" step=\""+String(tempstep,1)+"\" value=\""+String(cutOffTemp,1)+"\" style=\"width:auto\" onchange=\"uTI(this.value, '"+String(cutOffTempS)+"');\"><sup class=\"units\">&deg;C</sup></B><input id=\""+String(cutOffTempS)+"\" type=\"range\" min=\""+String(cutofflo,1)+"\" max=\""+String(cutoffhi,1)+"\" step=\""+String(tempstep,1)+"\" name=\""+String(PARAM_MESSAGE_cutOffTempSet)+"\" value=\""+String(cutOffTemp,1)+"\" style=\"width:50px\" onchange=\"uTI(this.value, 'T"+String(cutOffTempS)+"');\">";
    ptr+="<input type=\"submit\" style=\"width:45px\"></font>";
    ptr+="</td></tr>";

    ptr+="</table></form><br>";

    return ptr;
  }


  if (var=="scriptsectionreplace") {
    String ptr;
    String tmp;
    unsigned long int step=125;
    unsigned long int refreshtime = 9100;
    ptr="function uTI(e,n){document.getElementById(n).value=e};\n";
    tmp=String(uptimelink);  //spanid
    refreshtime+=step; ptr+="setInterval(function(){var e=new XMLHttpRequest;e.onreadystatechange=function(){4==this.readyState&&200==this.status&&(document.getElementById(\""+tmp+"\").innerHTML=this.responseText)},e.open(\"GET\",\"/"+tmp+"\",!0),e.send()},"+String(refreshtime/2)+");\n";
    tmp=String(tempCOThermometerS); refreshtime+=step; ptr+="setInterval(function(){var e=new XMLHttpRequest;e.onreadystatechange=function(){4==this.readyState&&200==this.status&&(document.getElementById(\""+tmp+"\").innerHTML=this.responseText)},e.open(\"GET\",\"/"+tmp+"\",!0),e.send()},"+String(refreshtime)+");\n";
    tmp=String(tempCOThermometerSetS); refreshtime+=step; ptr+="setInterval(function(){var e=new XMLHttpRequest;e.onreadystatechange=function(){4==this.readyState&&200==this.status&&(document.getElementById(\""+tmp+"\").innerHTML=this.responseText)},e.open(\"GET\",\"/"+tmp+"\",!0),e.send()},"+String(refreshtime)+");\n";
    tmp=String(dallThermometerS); refreshtime+=step; ptr+="setInterval(function(){var e=new XMLHttpRequest;e.onreadystatechange=function(){4==this.readyState&&200==this.status&&(document.getElementById(\""+tmp+"\").innerHTML=this.responseText)},e.open(\"GET\",\"/"+tmp+"\",!0),e.send()},"+String(refreshtime)+");\n";
    tmp=String(tempCORETThermometerS); refreshtime+=step; ptr+="setInterval(function(){var e=new XMLHttpRequest;e.onreadystatechange=function(){4==this.readyState&&200==this.status&&(document.getElementById(\""+tmp+"\").innerHTML=this.responseText)},e.open(\"GET\",\"/"+tmp+"\",!0),e.send()},"+String(refreshtime)+");\n";
    tmp=String(tempHWThermometerS); refreshtime+=step; ptr+="setInterval(function(){var e=new XMLHttpRequest;e.onreadystatechange=function(){4==this.readyState&&200==this.status&&(document.getElementById(\""+tmp+"\").innerHTML=this.responseText)},e.open(\"GET\",\"/"+tmp+"\",!0),e.send()},"+String(refreshtime)+");\n";
    tmp=String(tempHWThermometerSetS); refreshtime+=step; ptr+="setInterval(function(){var e=new XMLHttpRequest;e.onreadystatechange=function(){4==this.readyState&&200==this.status&&(document.getElementById(\""+tmp+"\").innerHTML=this.responseText)},e.open(\"GET\",\"/"+tmp+"\",!0),e.send()},"+String(refreshtime)+");\n";
    tmp=String(NEWS_lastTimeS); refreshtime+=step; ptr+="setInterval(function(){var e=new XMLHttpRequest;e.onreadystatechange=function(){4==this.readyState&&200==this.status&&(document.getElementById(\""+tmp+"\").innerHTML=this.responseText)},e.open(\"GET\",\"/"+tmp+"\",!0),e.send()},"+String(refreshtime)+");\n";
    tmp=String(tempROOMThermometerS); refreshtime+=step; ptr+="setInterval(function(){var e=new XMLHttpRequest;e.onreadystatechange=function(){4==this.readyState&&200==this.status&&(document.getElementById(\""+tmp+"\").innerHTML=this.responseText)},e.open(\"GET\",\"/"+tmp+"\",!0),e.send()},"+String(refreshtime)+");\n";
    tmp=String(tempROOMThermometerSetS); refreshtime+=step; ptr+="setInterval(function(){var e=new XMLHttpRequest;e.onreadystatechange=function(){4==this.readyState&&200==this.status&&(document.getElementById(\""+tmp+"\").innerHTML=this.responseText)},e.open(\"GET\",\"/"+tmp+"\",!0),e.send()},"+String(refreshtime)+");\n";
    return String(ptr);
  }
  #ifdef debug
//    Serial.print(F("End processor "));
//    Serial.println(var);
  #endif
  return String();

}
//******************************************************************************************
String uptimedana(unsigned long started_local) {
  String wynik = " ";
  if (started_local<1000) return "< 1 sek.";
  #ifdef debug
    Serial.print(F("Uptimedana: "));
  #endif
  unsigned long  partia = millis() - started_local;
  if (partia >= 24 * 60 * 60 * 1000 ) {
    unsigned long  podsuma = partia / (24 * 60 * 60 * 1000);
    partia -= podsuma * 24 * 60 * 60 * 1000;
    wynik += (String)podsuma + " dni ";

  }
  if (partia >= 60 * 60 * 1000 ) {
    unsigned long  podsuma = partia / (60 * 60 * 1000);
    partia -= podsuma * 60 * 60 * 1000;
    wynik += (String)podsuma + " godz. ";
  }
  if (partia >= 60 * 1000 ) {
    unsigned long  podsuma = partia / (60 * 1000);
    partia -= podsuma * 60 * 1000;
    wynik += (String)podsuma + " min. ";
    //Serial.println(podsuma);
  }
  if (partia >= 1 * 1000 ) {
    unsigned long  podsuma = partia / 1000;
    partia -= podsuma * 1000;
    wynik += (String)podsuma + " sek. ";
    //Serial.println(podsuma);
  }
  #ifdef debug
    Serial.println(wynik);
  #endif
  //wynik += (String)partia + "/1000";  //pomijam to wartosci <1sek
  return wynik;
}

#include <EEPROM.h>

#define CONFIG_VERSION "V00"sensitive_sizeS

// Where in EEPROM?
#define CONFIG_START 32

typedef struct
{
  char version[6]; // place to detect if settings actually are written
  unsigned int runNumber; // incremented on reboot
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
} configuration_type;

// with DEFAULT values!
configuration_type CONFIGURATION;

// load whats in EEPROM in to the local CONFIGURATION if it is a valid setting
bool loadConfig() {
  // is it correct?
  if (sizeof(CONFIGURATION)<1024) EEPROM.begin(1024); else EEPROM.begin(sizeof(CONFIGURATION)+128); //Size can be anywhere between 4 and 4096 bytes.
  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2] &&
      EEPROM.read(CONFIG_START + 3) == CONFIG_VERSION[3] &&
      EEPROM.read(CONFIG_START + 4) == CONFIG_VERSION[4]){

  // load (overwrite) the local configuration struct
    for (unsigned int i=0; i<sizeof(configuration_type); i++){
      *((char*)&CONFIGURATION + i) = EEPROM.read(CONFIG_START + i);
    }
    CONFIGURATION.runNumber++;
    runNumber = CONFIGURATION.runNumber;
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


    return true; // return 1 if config loaded
  }
  return false; // return 0 if config NOT loaded
}

// save the CONFIGURATION in to EEPROM
void saveConfig() {
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


  for (unsigned int i=0; i<sizeof(configuration_type); i++)
    EEPROM.write(CONFIG_START + i, *((char*)&CONFIGURATION + i));
  EEPROM.commit();
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