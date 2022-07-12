//String BASE_TOPIC = me_lokalizacja; //jest niepelna -brakuje kondygnacji
const String BASE_TOPIC = me_lokalizacja;

const String LOG_TOPIC = BASE_TOPIC + "/log";
const String WILL_TOPIC = BASE_TOPIC + "/Will";
const String IP_TOPIC = BASE_TOPIC + "/IP";
const String STATS_TOPIC = BASE_TOPIC + "/stats";

const String BASE_HA_TOPIC = "homeassistant";
const String ROOM_TEMP = "current_remote";
const String QOS = "0";
const String OT = "ot_";
const String BOILER = "boiler";
const String HOT_WATER = "domestic_hot_water";
const String ROOM_OTHERS = "room_other";
const String TEMPERATURE = "_temperature";
const String BOILER_TEMPERATURE = BOILER + TEMPERATURE;
// const String BOILER_MOD = BOILER+"-mode";   //tryb pracy
const String BOILER_TEMPERATURE_RET = BOILER + TEMPERATURE + "_return";
const String BOILER_TEMPERATURE_SETPOINT = BOILER + TEMPERATURE + "_setpoint";
const String BOILER_CH_STATE = BOILER + "_ch_state";
const String BOILER_SOFTWARE_CH_STATE_MODE = BOILER + "_software_ch_state_and_mode";
const String FLAME_STATE = "flame_state";
const String FLAME_LEVEL = "flame_level";
const String TEMP_CUTOFF = "temp_cutoff";
const String FLAME_W = "flame_used_energy";
const String FLAME_W_TOTAL = "flame_used_energy_total";

const String HOT_WATER_TEMPERATURE = HOT_WATER + TEMPERATURE;
const String HOT_WATER_TEMPERATURE_SETPOINT = HOT_WATER + TEMPERATURE + "_setpoint";
const String HOT_WATER_CH_STATE = HOT_WATER + "_dhw_state";
const String HOT_WATER_SOFTWARE_CH_STATE = HOT_WATER + "_software_dhw_state";

const String ROOM_OTHERS_TEMPERATURE = ROOM_TEMP + TEMPERATURE;
const String ROOM_OTHERS_TEMPERATURE_SETPOINT = ROOM_TEMP + TEMPERATURE + "_setpoint";
const String ROOM_OTHERS_PRESSURE = ROOM_OTHERS + "_pressure";

const String BOILER_TOPIC = BASE_TOPIC + "/" + BOILER + "/attributes";
const String HOT_WATER_TOPIC = BASE_TOPIC + "/" + HOT_WATER + "/attributes";
const String ROOM_OTHERS_TOPIC = BASE_TOPIC + "/" + ROOM_OTHERS + "/attributes";

const String ROOM_TEMP_SET_TOPIC = BASE_TOPIC + "/SET/" + ROOM_OTHERS_TEMPERATURE_SETPOINT + "/set"; // t
const String TEMP_SETPOINT_SET_TOPIC = BASE_TOPIC + "/SET/" + BOILER_TEMPERATURE_SETPOINT + "/set";  // sp
const String TEMP_CUTOFF_SET_TOPIC = BASE_TOPIC + "/SET/" + TEMP_CUTOFF + "/set";                    // cutOffTemp
const String STATE_DHW_SET_TOPIC = BASE_TOPIC + "/SET/" + HOT_WATER_SOFTWARE_CH_STATE + "/set";      // enableHotWater
const String MODE_SET_TOPIC = BASE_TOPIC + "/SET/" + BOILER_SOFTWARE_CH_STATE_MODE + "/set";         // 012 auto, heat, off ch
const String TEMP_DHW_SET_TOPIC = BASE_TOPIC + "/SET/" + HOT_WATER_TEMPERATURE_SETPOINT + "/set";    // dhwTarget
String COPUMP_GET_TOPIC = "COWoda0/switch/boilerroom/attributes";                      // temperatura outside avg NEWS
#define COPumpStatus_json "CO0_boilerroom_pump2CO"
#define WaterPumpStatus_json "CO0_boilerroom_pump1Water"

String NEWS_GET_TOPIC = "COWoda0/sensor/boilerroom/attributes";          // pompa CO status     IF CHANGE TOPIC -CHANGE CONFIGURATION VERSION !!!!!
#define NEWStemp_json "CO0_outside_temperature_Averange"


String ROOMS_F1_GET_TOPIC = "FLOORH1/sensor/room/attributes";          // pompa CO status value_json.FL2_room_temperature_0  FL2_room_temperature_setpoint_0
#define roomF1temp_json "FL1_room_temperature_0"
#define roomF1tempset_json "FL1_room_temperature_setpoint_0"
String ROOMS_F2_GET_TOPIC = "FLOORH2/sensor/room/attributes";          // pompa CO status
#define roomF2temp_json "FL2_room_temperature_0"
#define roomF2tempset_json "FL2_room_temperature_setpoint_0"

// logs topic
const String DIAGS = "diag";
const String DIAG_TOPIC = BASE_TOPIC + "/" + DIAGS + "/attributes";
const String DIAG_HA_TOPIC = BASE_HA_TOPIC + "/sensor/" + BASE_TOPIC + "/";
const String DIAG_HABS_TOPIC = BASE_HA_TOPIC + "/binary_sensor/" + BASE_TOPIC + "/";

const String LOGS = "log";
const String LOG_GET_TOPIC = BASE_TOPIC + "/" + DIAGS + "/" + LOGS;
const String INTEGRAL_ERROR_GET_TOPIC = DIAGS + "_" + "interr";
const String DIAGS_OTHERS_FAULT = DIAGS + "_" + "fault";
const String DIAGS_OTHERS_DIAG = DIAGS + "_" + "diagnostic";

//Homeassistant Autodiscovery topics
const String BOILER_HA_TOPIC = BASE_HA_TOPIC + "/sensor/" + BASE_TOPIC + "/" + BOILER;              //+"/state"
const String BOILER_HABS_TOPIC = BASE_HA_TOPIC + "/binary_sensor/" + BASE_TOPIC + "/" + BOILER;     //+"/state"
const String BOILER_HACLI_TOPIC = BASE_HA_TOPIC + "/climate/" + BASE_TOPIC + "/" + BOILER; //+"/state"

const String HOT_WATER_HA_TOPIC = BASE_HA_TOPIC + "/sensor/" + BASE_TOPIC + "/";                 //+"/state"
const String HOT_WATER_HABS_TOPIC = BASE_HA_TOPIC + "/binary_sensor/" + BASE_TOPIC + "/" + HOT_WATER;        //+"/state"
const String HOT_WATER_HACLI_TOPIC = BASE_HA_TOPIC + "/climate/" + BASE_TOPIC + "/" + HOT_WATER; //+"/state"

const String ROOM_OTHERS_HA_TOPIC = BASE_HA_TOPIC + "/sensor/" + BASE_TOPIC + "/" + ROOM_OTHERS;     //+"/state"
const String ROOM_OTHERS_HACLI_TOPIC = BASE_HA_TOPIC + "/climate/" + BASE_TOPIC + "/" + ROOM_OTHERS; //+"/state"



// setpoint topic
const String SETPOINT_OVERRIDE = "setpoint-override";
const String SETPOINT_OVERRIDE_SET_TOPIC = BASE_TOPIC + "/" + SETPOINT_OVERRIDE + "/set";     // op_override
const String SETPOINT_OVERRIDE_RESET_TOPIC = BASE_TOPIC + "/" + SETPOINT_OVERRIDE + "/reset"; //
