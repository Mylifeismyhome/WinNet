@echo off
title Compiling OpenSSL x86 Debug - [%date% %time%] - [7/8]

set CURPARTITION=%CD:~0,2%
set CURDIR=%CD%

set /p TOOL=<../../../Config/BUILDTOOLS_PATH
call "%TOOL%\Community\VC\Auxiliary\Build\vcvars32.bat"

%CURPARTITION%
cd %CURDIR%

set /p OPENSSL_SOURCE_DIRNAME=<openssl-version
cd %OPENSSL_SOURCE_DIRNAME%

nmake clean
perl Configure VC-WIN32 no-shared /MTD --debug --prefix=%CD%\..\..\bin\lib\x86\debug --openssldir=%CD%\..\..\bin\ssl
nmake
nmake test
nmake install

exit