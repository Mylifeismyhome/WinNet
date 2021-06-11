@echo off

title Installing WinNet Dependencies [%date% %time%]

openfiles >NUL 2>&1 
if NOT %ERRORLEVEL% EQU 0 goto NotAdmin 

echo -------------------------------------------
echo Started installation :: %date% %time%
echo -------------------------------------------

REM TOOLS REQUIRED - INSTALL THEM FIRST
REM --------------------------------------------------
mkdir Tools
cd %CD%\Tools

REM PERL
echo [+] Downloading PERL (5.32.1.1)
curl https://strawberryperl.com/download/5.32.1.1/strawberry-perl-5.32.1.1-64bit.msi -O strawberry-perl-5.32.1.1-64bit.msi
echo [+] Installing PERL (5.32.1.1)
call strawberry-perl-5.32.1.1-64bit.msi

REM NASM
echo [+] Downloading NASM (2.15.05)
curl https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/win64/nasm-2.15.05-installer-x64.exe -O nasm-2.15.05-installer-x64.exe
echo [+] Installing NASM (2.15.05)
call nasm-2.15.05-installer-x64.exe

cd ..
REM --------------------------------------------------

REM SET UP CONFIG
REM --------------------------------------------------
set /p TOOLS_PATH=[+] Enter Microsoft Visual Studio Build Tools Path: 
echo %TOOLS_PATH%>%CD%\Config\BUILDTOOLS_PATH

set /p PERL_PATH=[+] Enter PERL Path: 
echo %PERL_PATH%>%CD%\Config\PERL_PATH

REM Add NASM to Environment Scope
set /p NASM_PATH=[+] Enter NASM Path:
setx /M PATH "%PATH%;%NASM_PATH%"
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