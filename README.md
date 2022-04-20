
Own implementation
Integration opemntherm co gas boiler to homeassistant
Covers in backgroung autodiscovery homeassistant mqtt climate controls also
CO Heat depends on NEWS temperature -it starts only if outside temp is below some value (default 2degC) -this temp is get from mqtt broker as averange NEWS temp.
Control is also available by www ;)
Automode is control by room temp sensor and room temp target, heat mode is based on temperature CO pipe
In mqtt queue are double state value for dhw and double for boiler. eg. ch_state is centrah heat state with state reded from boiler, ch_state_software is software state which tells if we allow heat co. This is used for graphs -I use ch_state to make gradient on graph below Boiler temperature to see when boiler heated co, and flame status tells that boiler works.

Gets from mqtt broker
- temperature of NEWS (north, west south east averange temperature)
- state of working carbon heat co pump works -it disables hest to co


ToDo:
Get from mqtt averange min rooms sensors temperature and averange max rooms target temperature to automatically control heat.
