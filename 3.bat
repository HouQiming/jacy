@echo off
bootstrap\bin\win32_release\main main.jc
bin\win32\main --arch=win64 --rebuild main.jc
if errorlevel 1 goto end
bin\win64\main --arch=win32 --rebuild main.jc
if errorlevel 1 goto end
bin\win32\main --arch=win64 --rebuild main.jc
if errorlevel 1 goto end
bin\win64\main --arch=win32 --rebuild main.jc
if errorlevel 1 goto end
bin\win64\main test\test_gui.jc --rebuild --run
if errorlevel 1 goto end
echo "Cross bootstrap successful!"
:end
