@echo off
title Compiling CryptoPP x86 [RELEASE] - [%date% %time%] - [3/4]

set /p TOOL=<../../Config/BUILDTOOLS_PATH
call "%TOOL%\Community\VC\Auxiliary\Build\vcvars64.bat"

set pathMSBuild=%CD%\src\
rmdir %pathMSBuild%\Win32\Output\ /s /q
cd %pathMSBuild%
msbuild.exe cryptest.sln /property:Configuration=Release /property:Platform=Win32 /property:RuntimeLibrary=MultiThreadedDLL /target:cryptlib:Rebuild
cd ..

xcopy "%pathMSBuild%\Win32\Output\Release\cryptlib.lib" "%pathMSBuild%..\lib\release\x86\MD\" /s /i /q /y /c
xcopy "%pathMSBuild%\Win32\Output\Release\cryptlib.pdb" "%pathMSBuild%..\lib\release\x86\MD\" /s /i /q /y /c

rmdir %pathMSBuild%\Win32\Output\ /s /q
cd %pathMSBuild%
msbuild.exe cryptest.sln /property:Configuration=Release /property:Platform=Win32 /property:RuntimeLibrary=MultiThreaded /target:cryptlib:Rebuild
cd ..

xcopy "%pathMSBuild%\Win32\Output\Release\cryptlib.lib" "%pathMSBuild%..\lib\release\x86\MT\" /s /i /q /y /c
xcopy "%pathMSBuild%\Win32\Output\Release\cryptlib.pdb" "%pathMSBuild%..\lib\release\x86\MT\" /s /i /q /y /c

echo -------------------------------------------
echo Done compiling in Release Mode (x86)
echo -------------------------------------------

exit