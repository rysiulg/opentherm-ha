
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
Get from mqtt averange min rooms sensors temperature and averange max rooms target temperature to automatically control heat.


![image](https://user-images.githubusercontent.com/43485433/164272781-76c3ceb3-d773-43dc-bf9c-7fe399e58799.png)

HomeAssistaNT:

![image](https://user-images.githubusercontent.com/43485433/164273022-fdb00038-e56e-4a14-9947-2d9fbffecdc8.png)
![image](https://user-images.githubusercontent.com/43485433/164273183-3ccdbe4d-10ee-48ee-986c-9ee5c221af64.png)
