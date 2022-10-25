@echo off
title Compiling CryptoPP x86 [DEBUG] - [%date% %time%] - [1/4]

set /p TOOL=<../../Config/BUILDTOOLS_PATH
call "%TOOL%\Community\VC\Auxiliary\Build\vcvars32.bat"

set pathMSBuild=%CD%\src\
rmdir %pathMSBuild%\Win32\Output\ /s /q
cd %pathMSBuild%
msbuild.exe cryptlib.vcxproj /property:Configuration=Debug_MDD /property:Platform=Win32
cd ..

xcopy "%pathMSBuild%\Win32\Output\Debug_MDD\cryptlib.lib" "%pathMSBuild%..\lib\debug\x86\MDD\" /s /i /q /y /c
xcopy "%pathMSBuild%\Win32\Output\Debug_MDD\cryptlib.pdb" "%pathMSBuild%..\lib\debug\x86\MDD\" /s /i /q /y /c

rmdir %pathMSBuild%\Win32\Output\ /s /q
cd %pathMSBuild%
msbuild.exe cryptlib.vcxproj /property:Configuration=Debug /property:Platform=Win32
cd ..

xcopy "%pathMSBuild%\Win32\Output\Debug\cryptlib.lib" "%pathMSBuild%..\lib\debug\x86\MTD\" /s /i /q /y /c
xcopy "%pathMSBuild%\Win32\Output\Debug\cryptlib.pdb" "%pathMSBuild%..\lib\debug\x86\MTD\" /s /i /q /y /c

echo -------------------------------------------
echo Done compiling in Debug Mode (x86)
echo -------------------------------------------

exit