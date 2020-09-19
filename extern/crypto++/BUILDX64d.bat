@echo off
set /p Build=<../../Config/BUILDPATH
call "%Build%"

set pathMSBuild=%CD%\src\
cd %pathMSBuild%   
msbuild.exe cryptest.sln /property:Configuration=Debug /property:Platform=x64

xcopy "%pathMSBuild%\x64\Output\Debug\cryptlib.lib" "%pathMSBuild%..\lib\debug\x64\MDD\" /s /i /q /y /c
xcopy "%pathMSBuild%\x64\Output\Debug\cryptlib.pdb" "%pathMSBuild%..\lib\debug\x64\MDD\" /s /i /q /y /c

echo -------------------------------------------
echo Done compiling in Debug Mode (x64)
echo -------------------------------------------
exit