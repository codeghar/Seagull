@ECHO OFF
rem
rem "H248 Start Script Sample"
rem "Local env"
set RUN_DIR=REPLACETHIS

set PATH=%PATH%;%RUN_DIR%
set LD_LIBRARY_PATH=%RUN_DIR%
set RUN_DIR=%RUN_DIR%\h248-env\run

cd %RUN_DIR%
cls
mode 81,25
echo "Seagull H248 Server Sample Start"
seagull -conf ../config/conf.server.xml -dico ../config/h248-dictionary.xml -scen ../scenario/server.xml -log ../logs/server.log -llevel ET

pause
