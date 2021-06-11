@echo off

REM Select Mode
echo Mode:
echo 	1. Debug
echo		2. Debug_with_cipher
echo		3. Release
echo		4. Release_with_cipher
echo ---------------------------
set /p MODE=Select Mode: 
if %MODE%==1 (
	set MODE=Debug
) else if %MODE%==2 (
	set MODE=Debug_with_cipher
) else if %MODE%==3 (
	set MODE=Release
) else if %MODE%==4 (
	set MODE=Release_with_cipher
) else (
	echo Invalid MODE
	pause
	goto :end
)

REM Select Platform
echo Platform:
echo 	1. x86
echo		2. x64
echo ---------------------------
set /p PLATFORM=Select Platform: 
if %PLATFORM%==1 (
	set PLATFORM=x86
) else if %PLATFORM%==2 (
	set PLATFORM=x64
) else (
	echo Invalid Platform
	pause
	goto :end
)

echo -------------------------------------------
echo Start compiling WinNet Sandbox [%MODE%][%PLATFORM%] :: %date% %time%
echo -------------------------------------------

set /p TOOL=<../Config/BUILDTOOLS_PATH
call "%TOOL%\Community\VC\Auxiliary\Build\vcvars64.bat"

msbuild.exe Sandbox.sln /property:Configuration=%MODE% /property:Platform=%PLATFORM%

echo -------------------------------------------
echo Done compiling WinNet Sandbox [%MODE%][%PLATFORM%] :: %date% %time%
echo -------------------------------------------

pause

:end