@echo off
title Compiling OpenSSL x64 - [%date% %time%] - [2/2]

set CURPARTITION=%CD:~0,2%
set CURDIR=%CD%

set /p TOOL=<../../../Config/BUILDTOOLS_PATH
call "%TOOL%\Community\VC\Auxiliary\Build\vcvars64.bat"

%CURPARTITION%
cd %CURDIR%

REM Perl.exe
set /p PERL=<../../../Config/PERL_PATH

cd openssl-master

nmake clean
%PERL%\perl\bin\perl.exe Configure VC-WIN64A --debug --prefix=%CD%\..\BIN\DLL\x64\Debug --openssldir=%CD%\..\BIN\SSL
nmake
nmake install_sw

%PERL%\perl\bin\perl.exe Configure VC-WIN64A --prefix=%CD%\..\BIN\DLL\x64\Release --openssldir=%CD%\..\BIN\SSL
nmake
nmake install_sw

nmake clean
%PERL%\perl\bin\perl.exe Configure VC-WIN64A --debug --prefix=%CD%\..\BIN\Lib\x64\Debug --openssldir=%CD%\..\BIN\SSL no-shared
nmake
nmake install_sw

%PERL%\perl\bin\perl.exe Configure VC-WIN64A --prefix=%CD%\..\BIN\Lib\x64\Release --openssldir=%CD%\..\BIN\SSL no-shared
nmake
nmake install_sw

REM Copy all needed files
xcopy %CD%\..\BIN\Lib\x64\Debug\lib\libcrypto.lib %CD%\..\..\lib\x64\Debug\ /F /R /Y /I /J >> openssl_copy.log
xcopy %CD%\..\BIN\Lib\x64\Debug\lib\libssl.lib %CD%\..\..\lib\x64\Debug\ /F /R /Y /I /J >> openssl_copy.log
xcopy %CD%\..\BIN\Lib\x64\Debug\lib\ossl_static.pdb %CD%\..\..\lib\x64\Debug\ /F /R /Y /I /J >> openssl_copy.log

xcopy %CD%\..\BIN\Lib\x64\Release\lib\libcrypto.lib %CD%\..\..\lib\x64\Release\ /F /R /Y /I /J >> openssl_copy.log
xcopy %CD%\..\BIN\Lib\x64\Release\lib\libssl.lib %CD%\..\..\lib\x64\Release\ /F /R /Y /I /J >> openssl_copy.log
xcopy %CD%\..\BIN\Lib\x64\Release\lib\ossl_static.pdb %CD%\..\..\lib\x64\Release\ /F /R /Y /I /J >> openssl_copy.log

xcopy %CD%\..\BIN\Lib\x64\Release\include\openssl\* %CD%\..\..\include\openssl\ /F /R /Y /I /J >> openssl_copy.log

cd ..