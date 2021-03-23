@echo off

title Installing WinNet Dependencies [%date% %time%]

echo -------------------------------------------
echo Started installation :: %date% %time%
echo -------------------------------------------

REM TOOLS REQUIRED - INSTALL THEM FIRST
REM --------------------------------------------------
REM Microsoft Visual Studio Build Tools VC15
echo [+] Installing Visual Studio Build Tools
call %CD%\Tools\visualcppbuildtools_full.exe
echo [+] Installing NASM
call %CD%\Tools\nasm-2.14.02-installer-x64.exe
echo [+] Downloading PERL
curl https://strawberryperl.com/download/5.32.1.1/strawberry-perl-5.32.1.1-64bit.msi -O %CD%\Tools\strawberry-perl-5.32.1.1-64bit.msi
echo [+] Installing PERL
call %CD%\Tools\strawberry-perl-5.32.1.1-64bit.msi
REM --------------------------------------------------

REM SET UP CONFIG
REM --------------------------------------------------
set /p TOOLS_PATH=[+] Enter Microsoft Visual Studio Build Tools Path: 
echo %TOOLS_PATH%>%CD%\Config\BUILDTOOLS_PATH

set /p NASM_PATH=[+] Enter NASM Path: 
echo %NASM_PATH%>%CD%\Config\NASM_PATH

set /p PERL_PATH=[+] Enter PERL Path: 
echo %PERL_PATH%>%CD%\Config\PERL_PATH

set /p VCVARS_PATH=[+] Enter VCVars Path: 
echo %VCVARS_PATH%>%CD%\Config\VCVARS_PATH
REM --------------------------------------------------

REM Run installation
call %CD%\extern\OpenSSL\src\INSTALL.bat
call %CD%\BUILD.bat

echo -------------------------------------------
echo Completed installation :: %date% %time%
echo -------------------------------------------

pause