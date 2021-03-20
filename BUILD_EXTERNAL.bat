@echo off

REM Crypto++
cd %cd%\extern\crypto++\
call BUILD.bat

REM OpenSSL
cd %cd%\extern\OpenSSL\src
call BUILD.bat

exit