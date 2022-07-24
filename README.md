
Own implementation
Integration opemntherm co gas boiler to homeassistant
Covers in backgroung autodiscovery homeassistant mqtt climate controls also
CO Heat depends on NEWS temperature -it starts only if outside temp is below some value (default 2degC) -this temp is get from mqtt broker as averange NEWS temp.
Control is also available by www ;)
Default temp_NEWS have value 0 and default cutoff_NEWS is 2 so at defult heating is controlled, if from some reason communication is broke to mqqt to get new NEWS sensor than every temp_NEWS_interval_reduction_time_ms (default 30min) temp NEWS is reduced by 5% to finally get tempCutof value to enable heating.
Automode is control by room temp sensor and room temp target, heat mode is based on temperature CO pipe
In mqtt queue are double state value for dhw and double for boiler. eg. ch_state is centrah heat state with state reded from boiler, ch_state_software is software state which tells if we allow heat co. This is used for graphs -I use ch_state to make gradient on graph below Boiler temperature to see when boiler heated co, and flame status tells that boiler works.

Gets from mqtt broker
- temperature of NEWS (north, west south east averange temperature)
- state of working carbon heat co pump works -it disables hest to co


ToDo:

![image](https://user-images.githubusercontent.com/43485433/164272781-76c3ceb3-d773-43dc-bf9c-7fe399e58799.png)

HomeAssistaNT:

![image](https://user-images.githubusercontent.com/43485433/164273022-fdb00038-e56e-4a14-9947-2d9fbffecdc8.png)
![image](https://user-images.githubusercontent.com/43485433/164273183-3ccdbe4d-10ee-48ee-986c-9ee5c221af64.png)

Now updated version of http interface:



PLATFORM: Espressif 8266 (4.0.1) > WeMos D1 R2 and mini
HARDWARE: ESP8266 80MHz, 80KB RAM, 4MB Flash
PACKAGES:
 - framework-arduinoespressif8266 @ 3.30002.0 (3.0.2)
 - tool-esptool @ 1.413.0 (4.13)
 - tool-esptoolpy @ 1.30000.201119 (3.0.0)
 - tool-mklittlefs @ 1.203.210628 (2.3)
 - tool-mkspiffs @ 1.200.0 (2.0)
 - toolchain-xtensa @ 2.100300.210717 (10.3.0)
LDF: Library Dependency Finder -> https://bit.ly/configure-pio-ldf
LDF Modes: Finder ~ chain, Compatibility ~ soft
Found 43 compatible libraries
Scanning dependencies...
Dependency Graph
|-- ESPAsyncTCP-esphome @ 1.2.3
|-- ESPAsyncWebServer-esphome @ 2.1.0
|   |-- ESPAsyncTCP-esphome @ 1.2.3
|   |-- Hash @ 1.0
|   |-- ESP8266WiFi @ 1.0
|-- AsyncMqttClient-esphome @ 0.8.6
|   |-- ESPAsyncTCP-esphome @ 1.2.3
|-- ESP_DoubleResetDetector @ 1.3.1
|   |-- EEPROM @ 1.0
|   |-- LittleFS @ 0.1.0
|-- ESP8266 Influxdb @ 3.12.0
|   |-- ESP8266HTTPClient @ 1.2
|   |   |-- ESP8266WiFi @ 1.0
|   |-- ESP8266WiFi @ 1.0
|-- DallasTemperature @ 3.11.0
|   |-- OneWire @ 2.3.7
|-- OneWire @ 2.3.7
|-- OpenTherm Library @ 1.1.3
|-- ESP8266WiFi @ 1.0
|-- LittleFS @ 0.1.0
|-- ArduinoOTA @ 1.0
|   |-- ESP8266WiFi @ 1.0
|   |-- ESP8266mDNS @ 1.2
|   |   |-- ESP8266WiFi @ 1.0
|-- DNSServer @ 1.1.1
|   |-- ESP8266WiFi @ 1.0
|-- ESP8266HTTPClient @ 1.2
|   |-- ESP8266WiFi @ 1.0
|-- ESP8266mDNS @ 1.2
|   |-- ESP8266WiFi @ 1.0
|-- Hash @ 1.0
|-- Ticker @ 1.0
|-- EEPROM @ 1.0
Building in release mode
Compiling .pio\build\d1_mini\src\main.cpp.o
Linking .pio\build\d1_mini\firmware.elf
Retrieving maximum program size .pio\build\d1_mini\firmware.elf
Checking size .pio\build\d1_mini\firmware.elf
Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
RAM:   [=====     ]  54.7% (used 44840 bytes from 81920 bytes)
Flash: [======    ]  59.7% (used 623681 bytes from 1044464 bytes)
