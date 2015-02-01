@echo off
bootstrap\bin\win32_release\main main.jc
bin\win32\main --arch=win64 main.jc
if errorlevel 1 goto end
bin\win64\main %*
:end
