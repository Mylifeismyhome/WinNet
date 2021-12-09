@echo off
title Compile WinNet

setlocal enableextensions enabledelayedexpansion

REM Lists
set Configurations=DEBUG_DLL DEBUG_DLL_EXCLUDE_MYSQL DEBUG_LIB_MDD DEBUG_LIB_MDD_EXCLUDE_MYSQL DEBUG_LIB_MTD DEBUG_LIB_MTD_EXCLUDE_MYSQL RELEASE_DLL RELEASE_DLL_EXCLUDE_MYSQL RELEASE_LIB_MD RELEASE_LIB_MD_EXCLUDE_MYSQL RELEASE_LIB_MT RELEASE_LIB_MT_EXCLUDE_MYSQL
set Platforms=x86 x64

REM Select Mode
echo Mode:

set /a it=1
(for %%a in (%Configurations%) do ( 
    echo 	!it!. %%a
	set ConfigurationsIndexed[!it!]=%%a
	set /a it+=1
))

echo ---------------------------
set /p INPUT_MODE=Select Mode: 
if not defined ConfigurationsIndexed[%INPUT_MODE%] (
	echo Invalid Mode
	pause
	goto :end
) else (
	set MODE=!ConfigurationsIndexed[%INPUT_MODE%]!
)

REM Select Platform
echo Platform:

set /a it=1
(for %%a in (%Platforms%) do ( 
    echo 	!it!. %%a
	set PlatformIndexed[!it!]=%%a
	set /a it+=1
))

echo ---------------------------
set /p INPUT_PLATFORM=Select Platform: 
if not defined PlatformIndexed[%INPUT_PLATFORM%] (
	echo Invalid Platform
	pause
	goto :end
) else (
	set PLATFORM=!PlatformIndexed[%INPUT_PLATFORM%]!
)

echo -------------------------------------------
echo Start compiling WinNet [!MODE!][!PLATFORM!] :: %date% %time%
echo -------------------------------------------

set /p TOOL=<../../Config/BUILDTOOLS_PATH
call "%TOOL%\Community\VC\Auxiliary\Build\vcvars64.bat"

msbuild.exe Net.sln /property:Configuration=!MODE! /property:Platform=!PLATFORM!

echo -------------------------------------------
echo Done compiling WinNet [!MODE!][!PLATFORM!] :: %date% %time%
echo -------------------------------------------

pause

:end
endlocal