@echo off
title Compiling CryptoPP x86 [DEBUG] - [%date% %time%] - [1/4]

set /p TOOL=<../../Config/BUILDTOOLS_PATH
call "%TOOL%\Community\VC\Auxiliary\Build\vcvars32.bat"

set pathMSBuild=%CD%\src\
cd %pathMSBuild%   
msbuild.exe cryptest.sln /p:configuration=debug /property:Platform=Win32
cd ..

xcopy "%pathMSBuild%\Win32\Output\Debug\cryptlib.lib" "%pathMSBuild%..\lib\debug\x86\MDD\" /s /i /q /y /c
xcopy "%pathMSBuild%\Win32\Output\Debug\cryptlib.pdb" "%pathMSBuild%..\lib\debug\x86\MDD\" /s /i /q /y /c

echo -------------------------------------------
echo Done compiling in Debug Mode (x86)
echo -------------------------------------------