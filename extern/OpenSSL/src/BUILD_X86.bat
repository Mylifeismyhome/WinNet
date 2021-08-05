@echo off
title Compiling OpenSSL x86 - [%date% %time%] - [8/8]

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
perl Configure VC-WIN32 --prefix=%CD%\..\..\bin\lib\x86\release --openssldir=%CD%\..\..\bin\ssl no-shared
nmake
nmake install_sw

exit