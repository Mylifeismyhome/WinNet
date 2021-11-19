@echo off
title Compiling CryptoPP x64 [DEBUG] - [%date% %time%] - [2/4]

set /p TOOL=<../../Config/BUILDTOOLS_PATH
call "%TOOL%\Community\VC\Auxiliary\Build\vcvars64.bat"

set pathMSBuild=%CD%\src\
rmdir %pathMSBuild%\x64\Output\ /s /q
cd %pathMSBuild%
msbuild.exe cryptest.sln /property:Configuration=Debug /property:Platform=x64 /property:RuntimeLibrary=MultiThreadedDebugDLL /target:cryptlib:Rebuild
cd ..

xcopy "%pathMSBuild%\x64\Output\Debug\cryptlib.lib" "%pathMSBuild%..\lib\debug\x64\MDD\" /s /i /q /y /c
xcopy "%pathMSBuild%\x64\Output\Debug\cryptlib.pdb" "%pathMSBuild%..\lib\debug\x64\MDD\" /s /i /q /y /c

rmdir %pathMSBuild%\x64\Output\ /s /q
cd %pathMSBuild%
msbuild.exe cryptest.sln /property:Configuration=Debug /property:Platform=x64 /property:RuntimeLibrary=MultiThreadedDebug /target:cryptlib:Rebuild
cd ..

xcopy "%pathMSBuild%\x64\Output\Debug\cryptlib.lib" "%pathMSBuild%..\lib\debug\x64\MTD\" /s /i /q /y /c
xcopy "%pathMSBuild%\x64\Output\Debug\cryptlib.pdb" "%pathMSBuild%..\lib\debug\x64\MTD\" /s /i /q /y /c

echo -------------------------------------------
echo Done compiling in Debug Mode (x64)
echo -------------------------------------------

exit