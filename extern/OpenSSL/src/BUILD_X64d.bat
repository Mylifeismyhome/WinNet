@echo off
title Compiling OpenSSL x64 Debug - [%date% %time%] - [3/8]

set CURPARTITION=%CD:~0,2%
set CURDIR=%CD%

set /p TOOL=<../../../Config/BUILDTOOLS_PATH
call "%TOOL%\Community\VC\Auxiliary\Build\vcvars64.bat"

%CURPARTITION%
cd %CURDIR%

set /p OPENSSL_SOURCE_DIRNAME=<openssl-version
cd %OPENSSL_SOURCE_DIRNAME%

REM USE ALL CPU CORES
set CL=/MP

nmake clean
perl Configure VC-WIN64A --debug --prefix=%CD%\..\..\bin\lib\x64\debug --openssldir=%CD%\..\..\bin\ssl no-shared
nmake
nmake install_sw

exit