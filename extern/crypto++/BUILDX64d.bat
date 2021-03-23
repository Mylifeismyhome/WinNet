@echo off
title Compiling CryptoPP x64 [DEBUG] - [%date% %time%] - [2/4]

set /p TOOL=<../../Config/BUILDTOOLS_PATH
call "%TOOL%\vcbuildtools_msbuild.bat"

set pathMSBuild=%CD%\src\
cd %pathMSBuild%   
msbuild.exe cryptest.sln /property:Configuration=Debug /property:Platform=x64
cd ..

xcopy "%pathMSBuild%\x64\Output\Debug\cryptlib.lib" "%pathMSBuild%..\lib\debug\x64\MDD\" /s /i /q /y /c
xcopy "%pathMSBuild%\x64\Output\Debug\cryptlib.pdb" "%pathMSBuild%..\lib\debug\x64\MDD\" /s /i /q /y /c

echo -------------------------------------------
echo Done compiling in Debug Mode (x64)
echo -------------------------------------------