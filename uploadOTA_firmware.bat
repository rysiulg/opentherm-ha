set BINFILE=.pio/build/d1_mini/firmware.bin
set LOCALPORT=COM5
set OTAPORTx=10.48.18.28

"%USERPROFILE%\.platformio\penv\Scripts\python.exe" "%USERPROFILE%\.platformio\packages\framework-arduinoespressif8266\tools\espota.py" --debug --progress -i %OTAPORT% -f "%BINFILE%"



"%USERPROFILE%\AppData\Local\Arduino15\packages\esp8266\tools\python3\3.7.2-post1/python3" -I "%USERPROFILE%\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\3.0.2/tools/upload.py" --chip esp8266 --port "%LOCALPORT%" --baud 921600 --before default_reset --after hard_reset erase_flash write_flash 0x0 "%BINFILE%"

pause