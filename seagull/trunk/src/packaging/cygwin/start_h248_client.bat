@ECHO OFF
rem
rem "H248 Start Script Sample"
rem "Local env"
set RUN_DIR=REPLACETHIS

set PATH=%PATH%;%RUN_DIR%
set LD_LIBRARY_PATH=%RUN_DIR%
set RUN_DIR=%RUN_DIR%\h248-env\run

cd %RUN_DIR% 
mode 81,25
cls
echo "Seagull H248 Client Sample Start"
seagull -conf ../config/conf.client.xml -dico ../config/h248-dictionary.xml -scen ../scenario/client.xml -log ../logs/client.log -llevel ET

pause
