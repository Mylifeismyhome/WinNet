@echo off

title Compiling WinNet Dependencies [%date% %time%]

echo -------------------------------------------
echo Started :: %date% %time%
echo -------------------------------------------

REM Crypto++
cd %cd%\extern\crypto++\
call BUILD.bat
cd ../../

REM OpenSSL
cd %cd%\extern\OpenSSL\src
call BUILD.bat
cd ../../

echo -------------------------------------------
echo Completed :: %date% %time%
echo -------------------------------------------
pause