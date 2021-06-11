@echo off

REM Select Mode
echo Mode:
echo 	1. DEBUG_DLL
echo		2. DEBUG_LIB_MDD
echo		3. DEBUG_LIB_MTD
echo		4. RELEASE_DLL
echo		5. RELEASE_LIB_MD
echo		6. RELEASE_LIB_MT
echo		7. RELEASE_LIB_MT_VS13
echo ---------------------------
set /p MODE=Select Mode: 
if %MODE%==1 (
	set MODE=DEBUG_DLL
) else if %MODE%==2 (
	set MODE=DEBUG_LIB_MDD
) else if %MODE%==3 (
	set MODE=DEBUG_LIB_MTD
) else if %MODE%==4 (
	set MODE=RELEASE_DLL
) else if %MODE%==5 (
	set MODE=RELEASE_LIB_MD
) else if %MODE%==6 (
	set MODE=RELEASE_LIB_MT
) else if %MODE%==7 (
	set MODE=RELEASE_LIB_MT_VS13
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
echo Start compiling WinNet [%MODE%][%PLATFORM%] :: %date% %time%
echo -------------------------------------------

set /p TOOL=<../../Config/BUILDTOOLS_PATH
call "%TOOL%\Community\VC\Auxiliary\Build\vcvars64.bat"

msbuild.exe Net.sln /property:Configuration=%MODE% /property:Platform=%PLATFORM%

echo -------------------------------------------
echo Done compiling WinNet [%MODE%][%PLATFORM%] :: %date% %time%
echo -------------------------------------------

pause

:end