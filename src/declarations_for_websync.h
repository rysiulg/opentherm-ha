

//defined in common_functions 0,1,2
#define ASS_uptimedana 0
#define ASS_uptimedanaStr "uptimedana"
#define ASS_Statusy 1
#define ASS_StatusyStr "Statusy"
#define ASS_MemStats 2
#define ASS_MemStatsStr "MemStats"
#define ASS_temp_NEWS  3      // avg temperatura outside -getting this from mqtt topic as averange from 4 sensors North West East South
#define ASS_temp_NEWSStr "temp_NEWS"
#define ASS_tempCWU 4           //hot water temp
#define ASS_tempCWUStr "tempCWU"
#define ASS_dhwTarget 5         //hot water temp set
#define ASS_dhwTargetStr "sliderValue2"
#define ASS_cutOffTemp 6        // outside temp setpoint to cutoff heating co. CO heating is disabled if outside temp (temp_NEWS) is above this value
#define ASS_cutOffTempStr "sliderValue3"
#define ASS_roomtemp 7
#define ASS_roomtempStr "roomtemp"
#define ASS_roomtempSet 8
#define ASS_roomtempSetStr "sliderValue4"
#define ASS_AutoMode 9     //boiler mode NEWS_Temp, auto:NewsTemp+sp based on room tempset
#define ASS_AutoModeStr "boilermodewww"
#define ASS_EnableHeatingCO 10        //boiler heat for co heat/not
#define ASS_EnableHeatingCOStr "boilerwww"
#define ASS_EnableHotWater 11      //hotWater heat active/not
#define ASS_EnableHotWaterStr "boilerhwwww"
#define ASS_statusWaterActive 12
#define ASS_statusWaterActiveStr "statusWaterActive"
#define ASS_statusCHActive 13
#define ASS_statusCHActiveStr "statusCHActive"
#define ASS_statusFlameOn 14
#define ASS_statusFlameOnStr "statusFlameOn"
#define ASS_statusFault 15
#define ASS_statusFaultStr "statusFault"
#define ASS_lastNEWSSet 16      //last time NEWS get updated from mqtt
#define ASS_lastNEWSSetStr "lastNEWSSet"
#define ASS_retTemp  17        //Return temperature
#define ASS_retTempStr "retTemp"
#define ASS_tempBoiler 18
#define ASS_tempBoilerStr "tempBoiler"
#define ASS_UsedMedia 19           //Statistical data of used media
#define ASS_UsedMediaStr "UsedMedia"
#define ASS_tempBoilerSet 20     // boiler tempset on heat modetemp boiler set -mainly used in auto mode and for display on www actual temp
#define ASS_tempBoilerSetStr "sliderValue1"
#define ASS_ecoMode 21          //tryb pracy kondensacyjny -eco -temp grzania CO max 40st
#define ASS_ecoModeStr "ecoMode"
#define ASS_opcohi 22
#define ASS_opcohiStr "opcohi"
#define ASS_tempCWUhistereza 23
#define ASS_tempCWUhisterezaStr "sliderValue5"
#define ASS_calcCWU 24
#define ASS_calcCWUStr "calcCWU"
#define ASS_tempCOhistereza 25
#define ASS_tempCOhisterezaStr "sliderValue6"
#define ASS_calcCO 26
#define ASS_calcCOStr "calcCO"


#define ASS_Num (26 + 1)              //number of ASS definitions
#define numDecimalsWWW 1
#define cutoff_histereza 0.9        //histereza cutoff and roomtemp/roomtempset
#define dhwTargetStart 51           // domyslna temperatura docelowa wody uzytkowej
#define cutoffstartval 2
#define roomtempinitial 21
#define ecoModeMaxTemp 39           //max temp pracy kondensacyjnej
#define ecoModeDisabledMaxTemp 60
