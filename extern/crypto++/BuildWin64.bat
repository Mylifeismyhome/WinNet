@echo off
call "C:\Program Files (x86)\Microsoft Visual C++ Build Tools\vcbuildtools_msbuild.bat"
set pathMSBuild=%CD%\src\
cd %pathMSBuild%   
msbuild.exe cryptest.sln /property:Configuration=Release /property:Platform=x64

xcopy "%pathMSBuild%\x64\Output\Release\cryptlib.lib" "%pathMSBuild%..\lib\release\x64\MD\" /s /i /q /y /c
xcopy "%pathMSBuild%\x64\Output\Release\cryptlib.pdb" "%pathMSBuild%..\lib\release\x64\MD\" /s /i /q /y /c

echo -------------------------------------------
echo Done compiling in Release Mode (x64)
echo -------------------------------------------
exit