@echo off

title Compile WinNet

REM Select Mode
echo Mode:
echo 	1. DEBUG_DLL
echo		2. DEBUG_DLL_EXCLUDE_MYSQL
echo		3. DEBUG_LIB_MDD
echo		4. DEBUG_LIB_MDD_EXCLUDE_MYSQL
echo		5. DEBUG_LIB_MTD
echo		6. DEBUG_LIB_MTD_EXCLUDE_MYSQL
echo		7. RELEASE_DLL
echo		8. RELEASE_DLL_EXCLUDE_MYSQL
echo		9. RELEASE_LIB_MD
echo		10. RELEASE_LIB_MD_EXCLUDE_MYSQL
echo		11. RELEASE_LIB_MT
echo		12. RELEASE_LIB_MT_EXCLUDE_MYSQL
echo ---------------------------
set /p MODE=Select Mode: 
if %MODE%==1 (
	set MODE=DEBUG_DLL
) else if %MODE%==2 (
	set MODE=DEBUG_DLL_EXCLUDE_MYSQL
) else if %MODE%==3 (
	set MODE=DEBUG_LIB_MDD
) else if %MODE%==4 (
	set MODE=DEBUG_LIB_MDD_EXCLUDE_MYSQL
) else if %MODE%==5 (
	set MODE=DEBUG_LIB_MTD
) else if %MODE%==6 (
	set MODE=DEBUG_LIB_MTD_EXCLUDE_MYSQL
) else if %MODE%==7 (
	set MODE=RELEASE_DLL
) else if %MODE%==8 (
	set MODE=RELEASE_DLL_EXCLUDE_MYSQL
) else if %MODE%==9 (
	set MODE=RELEASE_LIB_MD
) else if %MODE%==10 (
	set MODE=RELEASE_LIB_MD_EXCLUDE_MYSQL
) else if %MODE%==11 (
	set MODE=RELEASE_LIB_MT
) else if %MODE%==12 (
	set MODE=RELEASE_LIB_MT_EXCLUDE_MYSQL
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