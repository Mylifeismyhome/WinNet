@echo off

title Compiling WinNet Dependencies [%date% %time%]

echo -------------------------------------------
echo Started Build :: %date% %time%
echo -------------------------------------------

set /A base_path = %cd%

REM Crypto++
cd %cd%\..\extern\crypto++\
call BUILD.bat
cd %base_path%

REM OpenSSL
cd %cd%\..\extern\OpenSSL\src\
call BUILD.bat
cd %base_path%

echo -------------------------------------------
echo Completed Build :: %date% %time%
echo -------------------------------------------