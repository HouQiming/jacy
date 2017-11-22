@echo off
echo *** Bootstrapping the compiler ***
bootstrap\bin\win32_release\main main.jc
echo *** Bootstrapping build system ***
bootstrap\bin\win32_release\main -awin32 -brelease test\pmjs.jc
copy test\bin\win32_release\pmjs.exe bin\win32\pmjs.exe
