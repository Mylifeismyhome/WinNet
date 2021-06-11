@echo off
title Compiling CryptoPP x86 [RELEASE] - [%date% %time%] - [3/4]

set /p TOOL=<../../Config/BUILDTOOLS_PATH
call "%TOOL%\BuildTools\VC\Auxiliary\Build\vcvars32.bat"

set pathMSBuild=%CD%\src\
cd %pathMSBuild%
msbuild.exe cryptest.sln /property:Configuration=Release /property:Platform=Win32
cd ..

xcopy "%pathMSBuild%\Win32\Output\Release\cryptlib.lib" "%pathMSBuild%..\lib\release\x86\MD\" /s /i /q /y /c
xcopy "%pathMSBuild%\Win32\Output\Release\cryptlib.pdb" "%pathMSBuild%..\lib\release\x86\MD\" /s /i /q /y /c

echo -------------------------------------------
echo Done compiling in Release Mode (x86)
echo -------------------------------------------