@echo off
title Compiling OpenSSL x86 Debug Shared - [%date% %time%] - [5/8]

set CURPARTITION=%CD:~0,2%
set CURDIR=%CD%

set /p TOOL=<../../../Config/BUILDTOOLS_PATH
call "%TOOL%\Community\VC\Auxiliary\Build\vcvars32.bat"

%CURPARTITION%
cd %CURDIR%

set /p OPENSSL_SOURCE_DIRNAME=<openssl-version
cd %OPENSSL_SOURCE_DIRNAME%

REM USE ALL CPU CORES
set CL=/MP

nmake clean
perl Configure VC-WIN32 --debug --prefix=%CD%\..\..\bin\dll\x86\debug --openssldir=%CD%\..\..\bin\ssl
nmake
nmake test
nmake install

exit