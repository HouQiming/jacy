@echo off
bootstrap\bin\win32_release\main main.jc
bin\win32\main --arch=win64 --build=release --rebuild main.jc
if errorlevel 1 goto end
bin\win64_release\main --arch=win32 --build=release --rebuild main.jc
if errorlevel 1 goto end
bin\win32_release\main --arch=win64 --build=release --rebuild main.jc
if errorlevel 1 goto end
bin\win64_release\main --arch=win32 --build=release --rebuild main.jc
if errorlevel 1 goto end
bin\win32_release\main --build=release --arch=win64 test\test_gui.jc --rebuild --run
if errorlevel 1 goto end
echo "Cross bootstrap successful! (release)"
:end
