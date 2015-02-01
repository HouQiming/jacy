@echo off
bootstrap\bin\win32_release\main main.jc
if errorlevel 1 goto end
bin\win32\main -vv %* >$log.txt
:end
