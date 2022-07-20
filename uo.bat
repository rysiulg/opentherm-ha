set BINFILE=src.ino.d1_mini.bin
set LOCALPORT=COM5

"%USERPROFILE%\AppData\Local\Arduino15\packages\esp8266\tools\python3\3.7.2-post1/python3" -I "%USERPROFILE%\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\3.0.2/tools/upload.py" --chip esp8266 --port "%LOCALPORT%" --baud 921600 --before default_reset --after hard_reset write_flash 0x0 "%BINFILE%"


pause