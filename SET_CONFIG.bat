@echo off
openfiles >NUL 2>&1 
if NOT %ERRORLEVEL% EQU 0 goto NotAdmin
 
set /p TOOLS_PATH=[+] Enter Microsoft Visual Studio Build Tools Path: 
echo %TOOLS_PATH%>%CD%\Config\BUILDTOOLS_PATH

set /p PERL_PATH=[+] Enter PERL Path: 
echo %PERL_PATH%>%CD%\Config\PERL_PATH

REM Add NASM to Environment Scope
set /p NASM_PATH=[+] Enter NASM Path: 
setx /M PATH "%PATH%;%NASM_PATH%"

goto End
:NotAdmin 
echo This command prompt is NOT ELEVATED (NOT Run as administrator). 
:End
pause