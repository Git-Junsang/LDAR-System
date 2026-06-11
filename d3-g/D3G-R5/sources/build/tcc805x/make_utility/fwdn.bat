@echo off

echo Start the FWDN V8 v1.4.12

echo Download fwdn.json file
.\fwdn.exe --fwdn .\prebuilt\fwdn.json

REM echo Download boot.json file
REM .\fwdn.exe -w .\prebuilt\boot.json

echo Download tcc8050_snor.cs.rom file
.\fwdn.exe -w .\tcc8050_snor.cs.rom --area die1 --storage snor