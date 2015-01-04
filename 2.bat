@echo off
bootstrap\bin\win32_release\main main.spap
if errorlevel 1 goto end
bin\win32\main %*
:end
