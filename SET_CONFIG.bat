@echo off

if not exist "Config" mkdir Config

set /p TOOLS_PATH=[+] Enter Microsoft Visual Studio Build Tools Path: 
echo %TOOLS_PATH%>%CD%\Config\BUILDTOOLS_PATH

REM Add PERL to Environment Scope
set /p PERL_PATH=[+] Enter PERL Path: 
setx /s PATH "%PATH%;%PERL_PATH%"

REM Add NASM to Environment Scope
set /p NASM_PATH=[+] Enter NASM Path: 
setx /s PATH "%PATH%;%NASM_PATH%"

pause