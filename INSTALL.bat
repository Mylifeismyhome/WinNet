@echo off

title Installing WinNet Dependencies [%date% %time%]

openfiles >NUL 2>&1 
if NOT %ERRORLEVEL% EQU 0 goto NotAdmin 

echo -------------------------------------------
echo Started installation :: %date% %time%
echo -------------------------------------------

cd /d %~dp0

REM TOOLS REQUIRED - INSTALL THEM FIRST
REM --------------------------------------------------
if not exist Tools mkdir Tools
cd Tools

REM PERL
echo [+] Downloading PERL (5.32.1.1)
if not exist strawberry-perl-5.32.1.1-64bit.msi curl https://strawberryperl.com/download/5.32.1.1/strawberry-perl-5.32.1.1-64bit.msi --output strawberry-perl-5.32.1.1-64bit.msi
echo [+] Installing PERL (5.32.1.1)
call strawberry-perl-5.32.1.1-64bit.msi

REM NASM
echo [+] Downloading NASM (2.15.05)
if not exist nasm-2.15.05-installer-x64.exe curl https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/win64/nasm-2.15.05-installer-x64.exe --output nasm-2.15.05-installer-x64.exe
echo [+] Installing NASM (2.15.05)
call nasm-2.15.05-installer-x64.exe

cd ..
REM --------------------------------------------------

REM SET UP CONFIG
REM --------------------------------------------------
if not exist Config mkdir Config

set /p TOOLS_PATH=[+] Enter Microsoft Visual Studio Path: (e. g. C:\Program Files (x86)\Microsoft Visual Studio\2019) 
echo %TOOLS_PATH%>%CD%\Config\BUILDTOOLS_PATH

REM Add PERL to Environment Scope
set /p PERL_PATH=[+] Enter PERL Path: (e. g. C:\Strawberry\perl\bin)
setx /M PATH "%PATH%;%PERL_PATH%"

REM Add NASM to Environment Scope
set /p NASM_PATH=[+] Enter NASM Path: (e. g. C:\Users\tobia\AppData\Local\bin\NASM)
setx /M "PATH" "%PATH%;%NASM_PATH%"
REM --------------------------------------------------

REM Run installation
call %CD%\extern\OpenSSL\src\INSTALL.bat
call %CD%\BUILD.bat

echo -------------------------------------------
echo Completed installation :: %date% %time%
echo -------------------------------------------

goto End
:NotAdmin 
echo This command prompt is NOT ELEVATED (NOT Run as administrator). 
:End
pause