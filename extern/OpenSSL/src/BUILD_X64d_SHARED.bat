@echo off
title Compiling OpenSSL x64 Debug Shared - [%date% %time%] - [1/8]

set CURPARTITION=%CD:~0,2%
set CURDIR=%CD%

set /p TOOL=<../../../Config/BUILDTOOLS_PATH
call "%TOOL%\Community\VC\Auxiliary\Build\vcvars64.bat"

%CURPARTITION%
cd %CURDIR%

REM Perl.exe
set /p PERL=<../../../Config/PERL_PATH

cd openssl-master

REM USE ALL CPU CORES
set CL=/MP

%PERL%\perl\bin\perl.exe Configure VC-WIN64A --debug --prefix=%CD%\..\BIN\DLL\x64\Debug --openssldir=%CD%\..\BIN\SSL
nmake
nmake install_sw

exit