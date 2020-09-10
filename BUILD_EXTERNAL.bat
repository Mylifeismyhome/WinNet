@echo off

REM Crypto++
cd %cd%\extern\crypto++\
start BUILD.bat

REM OpenSSL
cd %cd%\extern\OpenSSL\src
start BUILD.bat

exit