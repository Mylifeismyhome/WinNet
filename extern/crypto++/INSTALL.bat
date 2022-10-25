@echo off

if not exist "include" mkdir include
if not exist "include\cryptopp" mkdir include\cryptopp
xcopy "%CD%\src\*.h" include\cryptopp /s /i /q /y /c

exit