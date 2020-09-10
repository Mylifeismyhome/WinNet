@echo off
call "C:\Program Files (x86)\Microsoft Visual C++ Build Tools\vcbuildtools_msbuild.bat"
set pathMSBuild=%CD%\src\
cd %pathMSBuild%

msbuild.exe cryptest.sln /property:Configuration=Release

xcopy "%pathMSBuild%\Win32\Output\Release\cryptlib.lib" "%pathMSBuild%..\lib\release\x86\MD\" /s /i /q /y /c
xcopy "%pathMSBuild%\Win32\Output\Release\cryptlib.pdb" "%pathMSBuild%..\lib\release\x86\MD\" /s /i /q /y /c

echo -------------------------------------------
echo Done compiling in Release Mode (x86)
echo -------------------------------------------
exit