@ECHO OFF
rem
rem SIP Start Script Sample"
rem "Local env"
SET RUN_DIR=C:\Program Files\Seagull

set PATH=%PATH%;%RUN_DIR%
set LD_LIBRARY_PATH=%RUN_DIR%
set RUN_DIR=%RUN_DIR%\sip-env\run

cd %RUN_DIR%
cls
mode 81,25
echo "Seagull SIP Client Sample Start"
seagull -conf ../config/conf.client.xml -dico ../config/sip-dictionnary.xml -scen ../scenario/client.xml -log ../logs/client.log -llevel ET

pause
