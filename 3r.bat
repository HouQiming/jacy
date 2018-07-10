@echo off
echo *** Bootstrapping debug-version compiler ***
bootstrap\bin\win32_release\main main.jc
if errorlevel 1 goto end
echo *** Bootstrapping pmjs ***
bin\win32\main --rebuild --build=release test\pmjs.jc
if errorlevel 1 goto end
copy bin\win32_release\pmjs.exe bin\win32_release\pmjs_backup.exe
copy test\bin\win32_release\pmjs.exe bin\win32\
copy test\bin\win32_release\pmjs.exe bin\win32_release\
echo *** Switching to JS ***
bin\win32\pmjs runjs 3make_dist.js
:end
