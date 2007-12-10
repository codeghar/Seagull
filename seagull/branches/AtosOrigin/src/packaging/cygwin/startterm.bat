@ECHO OFF
rem
rem "Seagull Start Script Sample"

rem "Local env"
set RUN_DIR=REPLACETHIS
set LD_LIBRARY_PATH=%RUN_DIR%
set PATH=%PATH%;%RUN_DIR%

cd %RUN_DIR% 
cmd /k "mode 81,25 && cls && echo You can now run seagull by typing 'seagull'"

