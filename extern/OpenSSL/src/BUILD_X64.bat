@echo off
title Compiling OpenSSL x64 - [%date% %time%] - [2/2]

set /p VCVARS=<../../../Config/VCVARS_PATH
call "%VCVARS%vcvars64.bat"
call "%VCVARS%vcvarsall.bat" x64

REM Nasm.exe
set /p NASM=<../../../Config/NASM_PATH

REM Perl.exe
set /p PERL=<../../../Config/PERL_PATH

cd openssl-master

%NASM% clean
%PERL% Configure VC-WIN64A --debug --prefix=%CD%\..\BIN\DLL\x64\Debug --openssldir=%CD%\..\BIN\SSL
%NASM%
%NASM% install_sw

%PERL% Configure VC-WIN64A --prefix=%CD%\..\BIN\DLL\x64\Release --openssldir=%CD%\..\BIN\SSL
%NASM%
%NASM% install_sw

%NASM% clean
%PERL% Configure VC-WIN64A --debug --prefix=%CD%\..\BIN\Lib\x64\Debug --openssldir=%CD%\..\BIN\SSL no-shared
%NASM%
%NASM% install_sw

%PERL% Configure VC-WIN64A --prefix=%CD%\..\BIN\Lib\x64\Release --openssldir=%CD%\..\BIN\SSL no-shared
%NASM%
%NASM% install_sw

REM Copy all needed files
xcopy %CD%\..\BIN\Lib\x64\Debug\lib\libcrypto.lib %CD%\..\..\lib\x64\Debug\ /F /R /Y /I /J >> openssl_copy.log
xcopy %CD%\..\BIN\Lib\x64\Debug\lib\libssl.lib %CD%\..\..\lib\x64\Debug\ /F /R /Y /I /J >> openssl_copy.log
xcopy %CD%\..\BIN\Lib\x64\Debug\lib\ossl_static.pdb %CD%\..\..\lib\x64\Debug\ /F /R /Y /I /J >> openssl_copy.log

xcopy %CD%\..\BIN\Lib\x64\Release\lib\libcrypto.lib %CD%\..\..\lib\x64\Release\ /F /R /Y /I /J >> openssl_copy.log
xcopy %CD%\..\BIN\Lib\x64\Release\lib\libssl.lib %CD%\..\..\lib\x64\Release\ /F /R /Y /I /J >> openssl_copy.log
xcopy %CD%\..\BIN\Lib\x64\Release\lib\ossl_static.pdb %CD%\..\..\lib\x64\Release\ /F /R /Y /I /J >> openssl_copy.log

xcopy %CD%\..\BIN\Lib\x64\Release\include\openssl\* %CD%\..\..\include\openssl\ /F /R /Y /I /J >> openssl_copy.log

cd ..