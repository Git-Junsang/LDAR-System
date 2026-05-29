@ECHO OFF

set JSON_PATH=.\
set ROM_PATH=.\


fwdn.exe --fwdn %JSON_PATH%tcc805x_fwdn.json
fwdn.exe --write %JSON_PATH%tcc805x_boot.json
fwdn.exe --write %ROM_PATH%tcc805x_snor_boot.rom --area die1 --storage snor