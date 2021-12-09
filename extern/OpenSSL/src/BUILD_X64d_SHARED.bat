@echo off
title Compiling OpenSSL x64 Debug Shared - [%date% %time%] - [1/8]

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
perl Configure VC-WIN64A --debug --prefix=%CD%\..\..\bin\dll\x64\debug --openssldir=%CD%\..\..\bin\ssl
nmake
nmake test
nmake install

exit