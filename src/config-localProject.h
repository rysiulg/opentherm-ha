
#define me_lokalizacja "BOILER_GAZ" loctmp


// Master OpenTherm Shield pins configuration
const int OT_IN_PIN = D1;  // for Arduino, 4 for ESP8266 (D2), 21 for ESP32
const int OT_OUT_PIN = D2; // for Arduino, 5 for ESP8266 (D1), 22 for ESP32

// Temperature sensor pin
const int ROOM_TEMP_SENSOR_PIN = D0; // 0; //for Arduino, 14 for ESP8266 (D5), 18 for ESP32


#define boiler_rated_kWh 24
#define boiler_50_30 20.7     //boiler technical factory set data at working 50st and return 30st
#define boiler_50_30_ret 31     //boiler technical factory set data at working 50st and return 30st -assume that is that mode if rettemp i under 31degC
#define boiler_80_60 19.5     //boiler technical factory set data at working 80st and return 60st

String LastboilerResponseError = "\0";