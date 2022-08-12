//String BASE_TOPIC = me_lokalizacja; //jest niepelna -brakuje kondygnacji

#define BASETOPMAIN my_base_localisation
#ifndef loctmp
#define BASE_TOPIC BASETOPMAIN                   // zostawie stara nazwe        me_lokalizacja;
#else
#define BASE_TOPIC BASETOPMAIN loctmp
#endif
#undef BASETOPICMAIN


String LOG_TOPIC = String(BASE_TOPIC) + "/log";
String WILL_TOPIC = String(BASE_TOPIC) + "/Will";
String IP_TOPIC = String(BASE_TOPIC) + "/IP";
String STATS_TOPIC = String(BASE_TOPIC) + "/stats";
#define WILL_ONLINE "Online"
#define WILL_OFFLINE "Offline"

#define BASE_HA_TOPIC "homeassistant"
//Homeassistant Autodiscovery topics
#define HA_SENSORS_TOPIC String(BASE_HA_TOPIC) + "/sensor/" + String(BASE_TOPIC) + "/"
#define HA_SWITCH_TOPIC String(BASE_HA_TOPIC) + "/switch/" + String(BASE_TOPIC) + "/"
#define HA_BINARY_TOPIC String(BASE_HA_TOPIC) + "/binary_sensor/" + String(BASE_TOPIC) + "/"
#define HA_CLIMATE_TOPIC String(BASE_HA_TOPIC) + "/climate/" + String(BASE_TOPIC) + "/"

#define ROOM_TEMP "room_temp"
#define QOS 0
#define OT "ot_"
#define BOILER "CO"
#define HOT_WATER "CWU"
#define ROOM_OTHERS "room_other"
#define TEMPERATURE "_temp"
#define BOILER_TEMPERATURE String(BOILER) + String(TEMPERATURE)
// const String BOILER_MOD = BOILER+"-mode";   //tryb pracy
#define BOILER_TEMPERATURE_RET String(BOILER) + String(TEMPERATURE) + "_ret"
#define BOILER_TEMPERATURE_SETPOINT String(BOILER) + String(TEMPERATURE) + "_setp"
#define BOILER_CH_STATE String(BOILER) + "_ch_state"
#define BOILER_SOFTWARE_CH_STATE_MODE String(BOILER) + "_soft_ch_state_and_mode"
#define FLAME_STATE "flame_state"
#define FLAME_LEVEL "flame_level"
#define TEMP_CUTOFF "temp_cutoff"
#define FLAME_W "flame_used_energy"
#define FLAME_W_TOTAL "flame_used_energy_total"
#define FLAME_TIME_SEC_TOTAL "flame_working_total_secs"

#define FLAME_W_CH_TOTAL "flame_used_energy_co_total"
#define FLAME_TIME_SEC_CH_TOTAL "flame_working_co_total_secs"
#define FLAME_W_DHW_TOTAL "flame_used_energy_dhw_total"
#define FLAME_TIME_SEC_DHW_TOTAL "flame_working_dhw_total_secs"
#define ECOMODE_STATE "ecomode_state"

#define HOT_WATER_TEMPERATURE String(HOT_WATER) + String(TEMPERATURE)
#define HOT_WATER_TEMPERATURE_SETPOINT String(HOT_WATER) + String(TEMPERATURE) + "_setp"
#define HOT_WATER_CH_STATE String(HOT_WATER) + "_dhw_state"
#define HOT_WATER_SOFTWARE_CH_STATE String(HOT_WATER) + "_soft_dhw_state"

#define ROOM_OTHERS_TEMPERATURE String(ROOM_TEMP) + String(TEMPERATURE)
#define ROOM_OTHERS_TEMPERATURE_SETPOINT String(ROOM_TEMP) +  String(TEMPERATURE) + "_setp"
#define ROOM_OTHERS_PRESSURE String(ROOM_OTHERS) + "_press"

String FLAME_TOPIC = String(BASE_TOPIC) + "/" + String(BOILER) + "_Flame/attrib";
String BOILER_TOPIC = String(BASE_TOPIC) + "/" + String(BOILER) + "/attrib";
String BOILER_STATE_TOPIC = String(BASE_TOPIC) + "/" + String(BOILER) + "_State/attrib";

#define HOT_WATER_TOPIC String(BASE_TOPIC) + "/" + String(HOT_WATER) + "/attrib"
String ROOM_OTHERS_TOPIC =  String(BASE_TOPIC) + "/" + String(ROOM_OTHERS) + "/attrib";
String ROOM_OTHERS_json = "ot_current_remote_temperature_setpoint";

const String ROOM_TEMP_SET_TOPIC = String(BASE_TOPIC) + "/SET/" + String(ROOM_OTHERS_TEMPERATURE_SETPOINT) + "/set"; // t
const String TEMP_SETPOINT_SET_TOPIC = String(BASE_TOPIC) + "/SET/" + String(BOILER_TEMPERATURE_SETPOINT) + "/set";  // sp roomtempSet
const String TEMP_CUTOFF_SET_TOPIC = String(BASE_TOPIC) + "/SET/" + String(TEMP_CUTOFF) + "/set";                   // cutOffTemp
const String STATE_DHW_SET_TOPIC = String(BASE_TOPIC) + "/SET/" + String(HOT_WATER_SOFTWARE_CH_STATE) + "/set";      // enableHotWater
const String MODE_SET_TOPIC = String(BASE_TOPIC) + "/SET/" + String(BOILER_SOFTWARE_CH_STATE_MODE) + "/set";         // 012 auto, heat, off ch
const String TEMP_DHW_SET_TOPIC = String(BASE_TOPIC) + "/SET/" + String(HOT_WATER_TEMPERATURE_SETPOINT) + "/set";    // dhwTarget

#define COWODA0_SENSORS_TOPIC "COWoda0/sensor/boilerroom/attrib"
#define COWODA0_SWITCH_TOPIC "COWoda0/switch/boilerroom/attrib"
String COPUMP_GET_TOPIC = String(COWODA0_SWITCH_TOPIC);   // temperatura outside avg NEWS
String COPumpStatus_json = ("boilerroom_pump2CO");
String WaterPumpStatus_json = ("boilerroom_pump1Water");

String NEWS_GET_TOPIC = String(COWODA0_SENSORS_TOPIC);          // pompa CO status     IF CHANGE TOPIC -CHANGE CONFIGURATION VERSION !!!!!
String NEWStemp_json = ("outside_temp_Avg");


String ROOMS_F1_GET_TOPIC = ("FLOORH1/room/attrib_0");          // pompa CO status value_json.FL2_room_temperature_0  FL2_room_temperature_setpoint_0
String roomF1temp_json = ("room_temperature_0");
String roomF1tempset_json = ("room_temp_setp_0");
String ROOMS_F2_GET_TOPIC = ("FLOORH2/room/attrib_0");          // pompa CO status
String roomF2temp_json = ("room_temp_0");
String roomF2tempset_json = ("room_temperature_setp_0");

// logs topic
#define DIAGS "diag"
#define DIAG_TOPIC String(BASE_TOPIC) + "/" + String(DIAGS) + "/attrib"

#define LOGS "log"
#define LOG_GET_TOPIC String(BASE_TOPIC) + "/" + String(DIAGS) + "/" + String(LOGS)
#define INTEGRAL_ERROR_GET_TOPIC String(DIAGS) + "_" + "interr"
#define DIAGS_OTHERS_FAULT String(DIAGS) + "_" + "fault"
#define DIAGS_OTHERS_DIAG String(DIAGS) + "_" + "diagnostic"

// setpoint topic
#define SETPOINT_OVERRIDE  "setpoint-override"
#define SETPOINT_OVERRIDE_SET_TOPIC  String(BASE_TOPIC) + "/" + String(SETPOINT_OVERRIDE) + "/set"     // op_override
#define SETPOINT_OVERRIDE_RESET_TOPIC String(BASE_TOPIC) + "/" + String(SETPOINT_OVERRIDE) + "/reset" //
