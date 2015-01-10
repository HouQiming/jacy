@echo off
bootstrap\bin\win32_release\main main.spap
bin\win32\main --arch=win64 main.spap
if errorlevel 1 goto end
bin\win64\main %*
:end
