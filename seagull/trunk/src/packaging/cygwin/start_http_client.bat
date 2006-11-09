@ECHO OFF
rem
rem "HTTP Start Script Sample"
rem "Local env"
set RUN_DIR=REPLACETHIS

set PATH=%PATH%;%RUN_DIR%
set LD_LIBRARY_PATH=%RUN_DIR%
set RUN_DIR=%RUN_DIR%\http-env\run

cd %RUN_DIR% 
mode 81,25
cls
echo "Seagull HTTP Client Sample Start"
seagull -conf ../config/conf.client.xml -dico ../config/http-dictionary.xml -scen ../scenario/client.xml -log ../logs/client.log -llevel ET

pause
