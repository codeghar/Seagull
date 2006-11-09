@ECHO OFF
rem
rem "XCAP Start Script Sample"
rem "Local env"
set RUN_DIR=REPLACETHIS

set PATH=%PATH%;%RUN_DIR%
set LD_LIBRARY_PATH=%RUN_DIR%
set RUN_DIR=%RUN_DIR%\xcap-env\run

cd %RUN_DIR%
cls
mode 81,25
echo "Seagull XCAP Server Sample Start"
seagull -conf ../config/conf.server.xml -dico ../config/xcap-dictionary.xml -scen ../scenario/server.xml -log ../logs/server.log -llevel ET

pause
