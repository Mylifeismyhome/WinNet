@echo off
title Compiling OpenSSL x86 Shared - [%date% %time%] - [6/8]

set CURPARTITION=%CD:~0,2%
set CURDIR=%CD%

set /p TOOL=<../../../Config/BUILDTOOLS_PATH
call "%TOOL%\Community\VC\Auxiliary\Build\vcvars32.bat"

%CURPARTITION%
cd %CURDIR%

cd openssl-master

REM USE ALL CPU CORES
set CL=/MP

nmake clean
perl Configure VC-WIN32 --prefix=%CD%\..\BIN\DLL\x86\Release --openssldir=%CD%\..\BIN\SSL
nmake
nmake install_sw

exit