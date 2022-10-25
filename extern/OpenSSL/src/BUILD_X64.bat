@echo off
title Compiling OpenSSL x64 - [%date% %time%] - [4/8]

set CURPARTITION=%CD:~0,2%
set CURDIR=%CD%

set /p TOOL=<../../../Config/BUILDTOOLS_PATH
call "%TOOL%\Community\VC\Auxiliary\Build\vcvars64.bat"

%CURPARTITION%
cd %CURDIR%

set /p OPENSSL_SOURCE_DIRNAME=<openssl-version
cd %OPENSSL_SOURCE_DIRNAME%

nmake clean
perl Configure VC-WIN64A no-shared /MT --prefix=%CD%\..\..\bin\lib\x64\release --openssldir=%CD%\..\..\bin\ssl
nmake
nmake test
nmake install

exit