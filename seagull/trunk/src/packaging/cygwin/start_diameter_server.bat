@ECHO OFF
rem
rem "Diameter Start Script Sample"
rem "Local env"
set RUN_DIR=REPLACETHIS

set PATH=%PATH%;%RUN_DIR%
set LD_LIBRARY_PATH=%RUN_DIR%
set RUN_DIR=%RUN_DIR%\diameter-env\run

cd %RUN_DIR%
mount c:/ / 
mode 81,25
cls
echo "Seagull Diameter Server Sample Start"
seagull -conf ..\config\conf.server.xml -dico ..\config\base_cx.xml -scen ..\scenario\sar-saa.server.xml -log ..\logs\sar-saa.server.log -llevel ET

