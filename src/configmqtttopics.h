//String BASE_TOPIC = me_lokalizacja; //jest niepelna -brakuje kondygnacji
#define BASE_TOPIC "opentherm-thermostat" loctmp // zostawie stara nazwe        me_lokalizacja;

String LOG_TOPIC = String(BASE_TOPIC) + "/log";
String WILL_TOPIC = String(BASE_TOPIC) + "/Will";
String IP_TOPIC = String(BASE_TOPIC) + "/IP";
String STATS_TOPIC = String(BASE_TOPIC) + "/stats";


#define BASE_HA_TOPIC "homeassistant"
#define ROOM_TEMP "current_remote"
#define QOS "0"
#define OT "ot_"
#define BOILER "boiler"
#define HOT_WATER "domestic_hot_water"
#define ROOM_OTHERS "room_other"
#define TEMPERATURE "_temperature"
#define BOILER_TEMPERATURE String(BOILER) + String(TEMPERATURE)
// const String BOILER_MOD = BOILER+"-mode";   //tryb pracy
#define BOILER_TEMPERATURE_RET String(BOILER) + String(TEMPERATURE) + "_return"
#define BOILER_TEMPERATURE_SETPOINT String(BOILER) + String(TEMPERATURE) + "_setpoint"
#define BOILER_CH_STATE BOILER "_ch_state"
#define BOILER_SOFTWARE_CH_STATE_MODE String(BOILER) + "_software_ch_state_and_mode"
#define FLAME_STATE "flame_state"
#define FLAME_LEVEL "flame_level"
#define TEMP_CUTOFF "temp_cutoff"
#define FLAME_W "flame_used_energy"
#define FLAME_W_TOTAL "flame_used_energy_total"
#define FLAME_TIME_SEC_TOTAL "flame_working_secs_total"

#define FLAME_W_CH_TOTAL "flame_used_energy_central_heat_total"
#define FLAME_TIME_SEC_CH_TOTAL "flame_working_secs_central_heat_total"
#define FLAME_W_DHW_TOTAL "flame_used_energy_dhw_total"
#define FLAME_TIME_SEC_DHW_TOTAL "flame_working_secs_dhw_total"
#define ECOMODE_STATE "ecomode_state"

#define HOT_WATER_TEMPERATURE String(HOT_WATER) + String(TEMPERATURE)
#define HOT_WATER_TEMPERATURE_SETPOINT String(HOT_WATER) + String(TEMPERATURE) + "_setpoint"
#define HOT_WATER_CH_STATE String(HOT_WATER) + "_dhw_state"
#define HOT_WATER_SOFTWARE_CH_STATE String(HOT_WATER) + "_software_dhw_state"

#define ROOM_OTHERS_TEMPERATURE String(ROOM_TEMP) + String(TEMPERATURE)
#define ROOM_OTHERS_TEMPERATURE_SETPOINT String(ROOM_TEMP) +  String(TEMPERATURE) + "_setpoint"
#define ROOM_OTHERS_PRESSURE String(ROOM_OTHERS) + "_pressure"

#define BOILER_TOPIC String(BASE_TOPIC) + "/" + String(BOILER) + "/attributes"
#define HOT_WATER_TOPIC String(BASE_TOPIC) + "/" + String(HOT_WATER) + "/attributes"
String ROOM_OTHERS_TOPIC =  String(BASE_TOPIC) + "/" + String(ROOM_OTHERS) + "/attributes";
String ROOM_OTHERS_json = "ot_current_remote_temperature_setpoint";

const String ROOM_TEMP_SET_TOPIC = String(BASE_TOPIC) + "/SET/" + String(ROOM_OTHERS_TEMPERATURE_SETPOINT) + "/set"; // t
const String TEMP_SETPOINT_SET_TOPIC = String(BASE_TOPIC) + "/SET/" + String(BOILER_TEMPERATURE_SETPOINT) + "/set";  // sp roomtempSet
const String TEMP_CUTOFF_SET_TOPIC = String(BASE_TOPIC) + "/SET/" + String(TEMP_CUTOFF) + "/set";                   // cutOffTemp
const String STATE_DHW_SET_TOPIC = String(BASE_TOPIC) + "/SET/" + String(HOT_WATER_SOFTWARE_CH_STATE) + "/set";      // enableHotWater
const String MODE_SET_TOPIC = String(BASE_TOPIC) + "/SET/" + String(BOILER_SOFTWARE_CH_STATE_MODE) + "/set";         // 012 auto, heat, off ch
const String TEMP_DHW_SET_TOPIC = String(BASE_TOPIC) + "/SET/" + String(HOT_WATER_TEMPERATURE_SETPOINT) + "/set";    // dhwTarget

#define COWODA0_SENSORS_TOPIC "COWoda0/sensor/boilerroom/attributes"
#define COWODA0_SWITCH_TOPIC "COWoda0/switch/boilerroom/attributes"
String COPUMP_GET_TOPIC = String(COWODA0_SWITCH_TOPIC);   // temperatura outside avg NEWS
String COPumpStatus_json = ("CO0_boilerroom_pump2CO");
String WaterPumpStatus_json = ("CO0_boilerroom_pump1Water");

String NEWS_GET_TOPIC = String(COWODA0_SENSORS_TOPIC);          // pompa CO status     IF CHANGE TOPIC -CHANGE CONFIGURATION VERSION !!!!!
String NEWStemp_json = ("CO0_outside_temperature_Averange");


String ROOMS_F1_GET_TOPIC = ("FLOORH1/sensor/room/attributes");          // pompa CO status value_json.FL2_room_temperature_0  FL2_room_temperature_setpoint_0
String roomF1temp_json = ("FL1_room_temperature_0");
String roomF1tempset_json = ("FL1_room_temperature_setpoint_0");
String ROOMS_F2_GET_TOPIC = ("FLOORH2/sensor/room/attributes");          // pompa CO status
String roomF2temp_json = ("FL2_room_temperature_0");
String roomF2tempset_json = ("FL2_room_temperature_setpoint_0");

// logs topic
#define DIAGS "diag"
#define DIAG_TOPIC String(BASE_TOPIC) + "/" + String(DIAGS) + "/attributes"
#define DIAG_HA_TOPIC String(BASE_HA_TOPIC) + "/sensor/" + String(BASE_TOPIC) + "/"
#define DIAG_HABS_TOPIC String(BASE_HA_TOPIC) + "/binary_sensor/" + String(BASE_TOPIC) + "/"

#define LOGS "log"
#define LOG_GET_TOPIC String(BASE_TOPIC) + "/" + String(DIAGS) + "/" + String(LOGS)
#define INTEGRAL_ERROR_GET_TOPIC String(DIAGS) + "_" + "interr"
#define DIAGS_OTHERS_FAULT String(DIAGS) + "_" + "fault"
#define DIAGS_OTHERS_DIAG String(DIAGS) + "_" + "diagnostic"

//Homeassistant Autodiscovery topics
#define BOILER_HA_TOPIC String(BASE_HA_TOPIC) + "/sensor/" + String(BASE_TOPIC) + "/" + String(BOILER)             //+"/state"
#define BOILER_HABS_TOPIC String(BASE_HA_TOPIC) + "/binary_sensor/" + String(BASE_TOPIC) + "/" + String(BOILER)     //+"/state"
#define BOILER_HACLI_TOPIC String(BASE_HA_TOPIC) + "/climate/" + String(BASE_TOPIC) + "/" + String(BOILER) //+"/state"

#define HOT_WATER_HA_TOPIC String(BASE_HA_TOPIC) + "/sensor/" + String(BASE_TOPIC) + "/"                 //+"/state
#define HOT_WATER_HABS_TOPIC String(BASE_HA_TOPIC) + "/binary_sensor/" + String(BASE_TOPIC) + "/" + String(HOT_WATER)        //+"/state"
#define HOT_WATER_HACLI_TOPIC String(BASE_HA_TOPIC) + "/climate/" + String(BASE_TOPIC) + "/" + String(HOT_WATER) //+"/state"

#define ROOM_OTHERS_HA_TOPIC String(BASE_HA_TOPIC) + "/sensor/" + String(BASE_TOPIC) + "/" + String(ROOM_OTHERS)     //+"/state"
#define ROOM_OTHERS_HACLI_TOPIC String(BASE_HA_TOPIC) + "/climate/" + String(BASE_TOPIC) + "/" + String(ROOM_OTHERS) //+"/state"



// setpoint topic
#define SETPOINT_OVERRIDE  "setpoint-override"
#define SETPOINT_OVERRIDE_SET_TOPIC  String(BASE_TOPIC) + "/" + String(SETPOINT_OVERRIDE) + "/set"     // op_override
#define SETPOINT_OVERRIDE_RESET_TOPIC String(BASE_TOPIC) + "/" + String(SETPOINT_OVERRIDE) + "/reset" //
