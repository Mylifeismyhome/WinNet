@echo off
title Compiling OpenSSL x86 - [%date% %time%] - [1/2]

set CURPARTITION=%CD:~0,2%
set CURDIR=%CD%

set /p TOOL=<../../../Config/BUILDTOOLS_PATH
call "%TOOL%\vcbuildtools.bat" amd64

%CURPARTITION%
cd %CURDIR%

REM Nasm.exe
set /p NASM=<../../../Config/NASM_PATH

REM Perl.exe
set /p PERL=<../../../Config/PERL_PATH

cd openssl-master

nmake clean
%PERL%\perl.exe Configure VC-WIN32 --debug --prefix=%CD%\..\BIN\DLL\x86\Debug --openssldir=%CD%\..\BIN\SSL
nmake
nmake install_sw

nmake clean
%PERL%\perl.exe Configure VC-WIN32 --prefix=%CD%\..\BIN\DLL\x86\Release --openssldir=%CD%\..\BIN\SSL
nmake
nmake install_sw

nmake clean
%PERL%\perl.exe Configure VC-WIN32 --debug --prefix=%CD%\..\BIN\Lib\x86\Debug --openssldir=%CD%\..\BIN\SSL no-shared
nmake
nmake install_sw

nmake clean
%PERL%\perl.exe Configure VC-WIN32 --prefix=%CD%\..\BIN\Lib\x86\Release --openssldir=%CD%\..\BIN\SSL no-shared
nmake
nmake install_sw

REM Copy all needed files
xcopy %CD%\..\BIN\Lib\x86\Debug\lib\libcrypto.lib %CD%\..\..\lib\x86\Debug\ /F /R /Y /I /J >> openssl_copy.log
xcopy %CD%\..\BIN\Lib\x86\Debug\lib\libssl.lib %CD%\..\..\lib\x86\Debug\ /F /R /Y /I /J >> openssl_copy.log
xcopy %CD%\..\BIN\Lib\x86\Debug\lib\ossl_static.pdb %CD%\..\..\lib\x86\Debug\ /F /R /Y /I /J >> openssl_copy.log

xcopy %CD%\..\BIN\Lib\x86\Release\lib\libcrypto.lib %CD%\..\..\lib\x86\Release\ /F /R /Y /I /J >> openssl_copy.log
xcopy %CD%\..\BIN\Lib\x86\Release\lib\libssl.lib %CD%\..\..\lib\x86\Release\ /F /R /Y /I /J >> openssl_copy.log
xcopy %CD%\..\BIN\Lib\x86\Release\lib\ossl_static.pdb %CD%\..\..\lib\x86\Release\ /F /R /Y /I /J >> openssl_copy.log

xcopy %CD%\..\BIN\Lib\x86\Release\include\openssl\* %CD%\..\..\include\openssl\ /F /R /Y /I /J >> openssl_copy.log

cd ..