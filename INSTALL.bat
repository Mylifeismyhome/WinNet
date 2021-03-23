@echo off

title Installing WinNet Dependencies [%date% %time%]

echo -------------------------------------------
echo Started installation :: %date% %time%
echo -------------------------------------------

REM TOOLS REQUIRED - INSTALL THEM FIRST
REM --------------------------------------------------
REM Microsoft Visual Studio Build Tools VC15
echo [+] Downloading Visual Studio Build Tools
curl https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=&ved=2ahUKEwirkO7Q48bvAhXU8eAKHUMNAyAQFjAAegQIBBAD&url=https%3A%2F%2Fgo.microsoft.com%2Ffwlink%2F%3FLinkId%3D691126&usg=AOvVaw0geDw_h-TSCfzTMvYE2ZOw -O %CD%\Tools\visualcppbuildtools_full.exe
echo [+] Installing Visual Studio Build Tools
call %CD%\Tools\visualcppbuildtools_full.exe
echo [+] Downloading NASM (2.15.05)
curl https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/win64/nasm-2.15.05-installer-x64.exe -O nasm-2.15.05-installer-x64.exe
echo [+] Installing NASM (2.15.05)
call %CD%\Tools\nasm-2.15.05-installer-x64.exe
echo [+] Downloading PERL (5.32.1.1)
curl https://strawberryperl.com/download/5.32.1.1/strawberry-perl-5.32.1.1-64bit.msi -O %CD%\Tools\strawberry-perl-5.32.1.1-64bit.msi
echo [+] Installing PERL (5.32.1.1)
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