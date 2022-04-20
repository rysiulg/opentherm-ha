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
  <meta http-equiv="refresh" content="30">
  <title>%me_lokalizacja%</title>

  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">

  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 2.1rem; }
    p { font-size: 2.2rem; }
    .units { font-size: 1.1rem; }
    .dht-labels{
      font-size: 1.3rem;
      vertical-align:middle;
      padding-bottom: 8px;
    }
  </style>
</head>
<body>
  <div class='s'><svg version='1.1' width="75px" height="75px" id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>
  <h2>
  ESP CO Server %ver%</h2>
  <p>
  <sup class="units">Uptime <b><span id="%uptime%">%uptimewart%</span></B></sup>
  <br/>
<sup class="units">%dane%</sup>
<font size="5" color="pink">Strona odświeża się co ok. 30sekund powodując skasowanie niezapisanych wartości w polach "ZMIEŃ NA"</font><br>
  </p>
  <form action="/get">
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i>
    <span class="dht-labels">Temperatura Zewnątrz Średnia NEWS:</span><B>
    <span id="%dallThermometerS%">%NEWS%</span>
    <sup class="units">&deg;C</sup></B>
    <font size="4" color="blue"> Aktualne od: <B><span id="%NEWS_lastTimeS%">%NEWS_lastTime%</span></B> </font>
  </p>
  <p>
  <LABEL FOR="BOILMOD">Tryb pracy CO:</LABEL>
  <INPUT TYPE="Radio" ID="BOILMOD" Name="boilermodewww" Value="2" %BOILMOD2%>Automatyczny
  <INPUT TYPE="Radio" ID="BOILMOD" Name="boilermodewww" Value="1" %BOILMOD1%>Grzanie/Wyłączone
  </p><p>
  <LABEL FOR="HWMOD">Tryb pracy CWU:</LABEL>
  <INPUT TYPE="Radio" ID="HWMOD" Name="boilerhwwww" Value="1" %HWMOD1%>Grzanie
  <INPUT TYPE="Radio" ID="HWMOD" Name="boilerhwwww" Value="0" %HWMOD0%>Wyłączone
  </p>

  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i>
    <span class="dht-labels">Temperatura Boilera i do CO</span><B>
    <span id="%tempCOThermometerS%">%TH%</span>
    <sup class="units">&deg;C</sup></B>

    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i>
    <span class="dht-labels">Temperatura do CO USTAW: </span><B>
    <span id="%tempCOThermometerSetS%">%THCOSET%</span>
    <sup class="units">&deg;C</sup></B>
    <font size="4" color="red">
    </B>ZMIEŃ NA: <input type="text" name="tempCOset" value="%THCOSET%" style="width:50px">
    <input type="submit" style="width:45px"></font
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i>
    <span class="dht-labels">Temperatura progu grzania CO gdy średnia NEWS: &lt; </span><B>
    <span id="%cutOffTempS%">%cutOffTempVAL%</span>
    <sup class="units">&deg;C</sup></B>
    <font size="4" color="red">
    </B>ZMIEŃ NA: <input type="text" name="tempCutOffset" value="%cutOffTempVAL%" style="width:50px">
    <input type="submit" style="width:45px"></font>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i>
    <span class="dht-labels">Temperatura Powrotu:</span><B>
    <span id="%tempCORETThermometerS%">%THRET%</span>
    <sup class="units">&deg;C</sup></B>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i>
    <span class="dht-labels">Temperatura WODY:</span><B>
    <span id="%tempHWThermometerS%">%DHW%</span>
    <sup class="units">&deg;C</sup></B>

    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i>
    <span class="dht-labels">Temperatura Wody Docel.: </span><B>
    <span id="%tempHWThermometerSetS%">%DHWSET%</span>
    <sup class="units">&deg;C</sup></B>
    <font size="4" color="red">
    </B>ZMIEŃ NA: <input type="text" name="tempHWset" value="%DHWSET%" style="width:50px">
    <input type="submit" style="width:45px"></font>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i>
    <span class="dht-labels">Temperatura Pokoju</span><B>
    <span id="%tempROOMThermometerS%">%ROOM%</span>
    <sup class="units">&deg;C</sup></B>

    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i>
    <span class="dht-labels">Temperatura Pokoju Docel.: </span><B>
    <span id="%tempROOMThermometerSetS%">%ROOMSET%</span>
    <sup class="units">&deg;C</sup></B>
    <font size="4" color="red">
    </B>ZMIEŃ NA: <input type="text" name="tempROOMset" value="%ROOMSET%" style="width:50px">
    <input type="submit" style="width:45px"></font>
  </p>

  <form>
  <br><br><p>
    <span class="units">%stopkawebsite%</span>
  </p>
</body>
<script>
  setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById("%dallThermometerS%").innerHTML = this.responseText; }};
    xhttp.open("GET", "/%dallThermometerS%", true); xhttp.send();
    xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById("%tempCOThermometerS%").innerHTML = this.responseText; }};
    xhttp.open("GET", "/%tempCOThermometerS%", true); xhttp.send();
    xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById("%tempCOThermometerSetS%").innerHTML = this.responseText; }};
    xhttp.open("GET", "/%tempCOThermometerSetS%", true); xhttp.send();
    xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById("%tempCORETThermometerS%").innerHTML = this.responseText; }};
    xhttp.open("GET", "/%tempCORETThermometerS%", true); xhttp.send();
    xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById("%tempHWThermometerS%").innerHTML = this.responseText; }};
    xhttp.open("GET", "/%tempHWThermometerS%", true); xhttp.send();
    xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById("%tempHWThermometerSetS%").innerHTML = this.responseText; }};
    xhttp.open("GET", "/%tempHWThermometerSetS%", true); xhttp.send();
    xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById("%cutOffTempS%").innerHTML = this.responseText; }};
    xhttp.open("GET", "/%cutOffTempS%", true); xhttp.send();
    xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById("%NEWS_lastTimeS%").innerHTML = this.responseText; }};
    xhttp.open("GET", "/%NEWS_lastTimeS%", true); xhttp.send();
    xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById("%tempROOMThermometerS%").innerHTML = this.responseText; }};
    xhttp.open("GET", "/%tempROOMThermometerS%", true); xhttp.send();
    xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById("%tempROOMThermometerSetS%").innerHTML = this.responseText; }};
    xhttp.open("GET", "/%tempROOMThermometerSetS%", true); xhttp.send();
  }, 10000 ) ;
  setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById("links").innerHTML = this.responseText;}};
    xhttp.open("GET", "/links", true); xhttp.send();
    xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById("%uptime%").innerHTML = this.responseText;}};
    xhttp.open("GET", "/%uptime%", true); xhttp.send();
  }, 11000 ) ;
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
  #ifdef debug
  Serial.printf("Progress: %d%%\n", (prg * 100) / content_len);
  #endif
  WebSerial.print("Progress: ");
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
    Serial.println("Update");
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
      Serial.println("Update complete");
      WebSerial.println("Update complete");
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
    Serial.println("subWerbServers...");
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
  webserver.on("/"uptimelink , HTTP_GET, [](AsyncWebServerRequest * request) {
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





  webserver.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String message;
      if (request->hasParam(PARAM_MESSAGE_tempCOset)) { //tempCOset
          message = request->getParam(PARAM_MESSAGE_tempCOset)->value();
          message.trim();
          message.replace(",",".");
          float liczba = message.toFloat();
          if (isnan(liczba) || !isValidNumber(message)) {
            Serial.println("Liczba not a valid number, ignoring...");
            WebSerial.println("Liczba not a valid number, ignoring...");
          }
          else {
            if (liczba>opcohi) liczba=opcohi;
            if (liczba<opcolo) liczba=opcolo;
            tempBoilerSet = liczba;
            op_override = liczba;
 //           op = liczba;
            message = String(liczba);
            WebSerial.print("WebReceived change Boiler CO Set to: ");
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
            Serial.println("Liczba not a valid number, ignoring...");
            WebSerial.println("Liczba not a valid number, ignoring...");
          }
          else {
            if (liczba>cutoffhi) liczba=cutoffhi;
            if (liczba<cutofflo) liczba=cutofflo;
            cutOffTemp = liczba;
            message = String(liczba);
            WebSerial.print("WebReceived change TempCutOff Set to: ");
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
            Serial.println("Liczba not a valid number, ignoring...");
            WebSerial.println("Liczba not a valid number, ignoring...");
          }
          else {
            if (liczba>ophi) liczba=ophi;
            if (liczba<oplo) liczba=oplo;
            dhwTarget = liczba;
            message = String(liczba);
            WebSerial.print("WebReceived change Boiler CO Set to: ");
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
            Serial.println("Liczba not a valid number, ignoring...");
            WebSerial.println("Liczba not a valid number, ignoring...");
          }
          else {
            if (liczba>roomtemphi) liczba=roomtemphi;
            if (liczba<roomtemplo) liczba=roomtemplo;
            sp = liczba;
            message = String(liczba);
            WebSerial.print("WebReceived change Boiler CO Set to: ");
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
            Serial.println("Liczba not a valid number, ignoring...");
            WebSerial.println("Liczba not a valid number, ignoring...");
          }
          else {
            if (liczba==2) automodeCO=true;
            if (liczba==1) automodeCO=false;  //mode heat and off is controlled by outside temp cutoff
            message = String(liczba);
            WebSerial.print("WebReceived change Boiler Mode CO to: ");
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
            Serial.println("Liczba not a valid number, ignoring...");
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
	Serial.println("HTTP server started");
}


String processor(const String var) {


  #ifdef debug
//    Serial.print(F("Start processor: "));
//    Serial.println(var);
  #endif
  if (var == "ver") {
    String a = "v. ";
    a += me_version;
    a += "<br>";
    a += client.connected()? "MQTT Podłączony: "+String(mqtt_server)+":"+String(mqtt_port) : "MQTT Rozłączony: "+String(mqtt_server)+":"+String(mqtt_port) ;  //1 conn, 0 not conn


    return a;
  }
  if (var == "NEWS") {
    return String(temp_NEWS);
  }
  if (var == "NEWS_lastTime") {
    return String(uptimedana(temp_NEWS_count*temp_NEWS_interval_reduction_time_ms+lastNEWSSet));  //last update time
  }
  if (var == "NEWS_lastTimeS") {
    return String("NEWS_lastTimeS");  //last update time
  }
    if (var == dallThermometerS) {
    return String(dallThermometerS);
  }
  if (var == "TH") {
    return String(tempBoiler,1);
  }
  if (var == "THCOSET") {
    return String(tempBoilerSet,1);
  }
  if (var == tempCOThermometerS) {
    return String(tempCOThermometerS);
  }
  if (var == tempCOThermometerSetS) {
    return String(tempCOThermometerSetS);
  }
  if (var == "tempCOset") {
    return String(tempCOset);
  }
  if (var == tempCORETThermometerS) {
    return String(tempCORETThermometerS);
  }
  if (var == "THRET") {
    return String(retTemp,1);
  }
  if (var == tempHWThermometerS) {
    return String(tempHWThermometerS);
  }
  if (var == tempHWThermometerSetS) {
    return String(tempHWThermometerSetS);
  }
  if (var == "tempHWset") {
    return String(dhwTarget,1);
  }
  if (var == "DHW") {
    return String(tempCWU,1);
  }
  if (var == "DHWSET") {
    return String(dhwTarget,1);
  }
  if (var == cutOffTempSet) {
    return String(cutOffTempSet);
  }
  if (var == cutOffTempS) {
    return String(cutOffTempS);
  }
  if (var == "cutOffTempVAL") {
    return String(cutOffTemp,1);
  }

  if (var == "BOILMOD2") {
    return String(automodeCO?"Checked":"");
  }
  if (var == "BOILMOD1") {
    return String(automodeCO?"":"Checked");
  }
  if (var == "HWMOD1") {
    return String(enableHotWater?"Checked":"");
  }
  if (var == "HWMOD0") {
    return String(enableHotWater?"":"Checked");
  }


  if (var == "ROOM") {
    return String(roomtemp,1);
  }
  if (var == "ROOMSET") {
    return String(sp,1);
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
//    if ("isadslinitialised="") a += "Tak"; else a += "Nie";

//    if (ownother.length()>0) {
//      a += " Własny string ownother: ";
//      a += ownother;
//    }
//    a += " GASlast: ";
//    a += gasCOMeterS->getlast();
    #ifdef debug
      Serial.println(F("Raport Hosta po read_eprom: "));
    #endif
    return String(a);
  }
  if (var == "uptime") {
    return String(uptimelink);
  }
//  if (var == "json") {
//    return String(savedjson);
//  }
  if (var == "uptimewart") {
    return String(uptimedana(started));
  }
  if (var == "me_lokalizacja") {
    return String(me_lokalizacja);
  }
  if (var == "stopkawebsite") {
    String ptr;
      ptr = "</span>&nbsp;";
      if (status_FlameOn) {
        ptr += "<i class='fas fa-fire' style='color: red'></i>";
        ptr += "<span class='dht-labels'>Płomień aktywny. Moc palnika: </span><B>";
        ptr += String(flame_level,0);
        ptr += "<sup class=\"units\">&#37;</sup></B>";
        ptr += "<br>";
      }
      if (status_Fault) ptr += "<span class='dht-labels'><B>!!!!!!!!!!!!!!!!! status_Fault !!!!!!!<br></B></span>";
      if (heatingEnabled) ptr += "<span class='dht-labels'><B>Ogrzewanie CO włączone<br></B></span>";
      if (status_CHActive) ptr += "<font color=\"red\"><span class='dht-labels'><B>Piec w trybie aktywnego grzania CO<br></B></span></font>";
      if (enableHotWater) ptr += "<span class='dht-labels'><B>Podgrzewanie WODY CWU włączone<br></B></span>";
      if (status_WaterActive) ptr += "<font color=\"red\"><span class='dht-labels'><B>Piec w trybie aktywnego podgrzewania CWU wody użytkowej<br></B></span></font>";

      if (status_Cooling) ptr += "<font color=\"orange\"><span class='dht-labels'><B>Tryb Chłodzenia<br></B></span></font>";
      if (status_Diagnostic) ptr += "<font color=\"darkred\"><span class='dht-labels'><B>Tryb Diagnostyczny<br></B></span></font>";
      if (CO_PumpWorking) ptr += "<font color=\"blue\"><span class='dht-labels'><B>Pompa CO pieca węglowego aktywna -dezaktywuje grzanie;)<br></B><br></span></font>";
      ptr += "<br><span class='units'><a href='/update'>Aktualizacja</a> &nbsp; &nbsp;&nbsp; <a href='/webserial'>Serial Terminal</a>&nbsp;";
      ptr += "<br>&copy; ";
      ptr += stopka;
      ptr += "<br>";
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

#define CONFIG_VERSION "VER03"

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
  char ssid[32];
  char pass[32];
  char mqtt_server[32];
  char mqtt_user[32];
  char mqtt_password[32];
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