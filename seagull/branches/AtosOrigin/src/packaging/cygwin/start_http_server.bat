@ECHO OFF
rem
rem "HTTP Start Script Sample"
rem "Local env"
set RUN_DIR=REPLACETHIS

set PATH=%PATH%;%RUN_DIR%
set LD_LIBRARY_PATH=%RUN_DIR%
set RUN_DIR=%RUN_DIR%\http-env\run

cd %RUN_DIR%
cls
mode 81,25
echo "Seagull HTTP Server Sample Start"
seagull -conf ../config/conf.server.xml -dico ../config/http-dictionary.xml -scen ../scenario/server.xml -log ../logs/server.log -llevel ET

pause
