@echo off
set /p BuildOpenssl=<../../../Config/BUILDPATH_OPENSSL
call "%BuildOpenssl%vcvars64.bat"
call "%BuildOpenssl%vcvarsall.bat" x64

cd openssl-master

nmake clean
perl Configure VC-WIN64A --debug --prefix=%CD%\..\BIN\DLL\x64\Debug --openssldir=%CD%\..\BIN\SSL
nmake
nmake install_sw

perl Configure VC-WIN64A --prefix=%CD%\..\BIN\DLL\x64\Release --openssldir=%CD%\..\BIN\SSL
nmake
nmake install_sw

nmake clean
perl Configure VC-WIN64A --debug --prefix=%CD%\..\BIN\Lib\x64\Debug --openssldir=%CD%\..\BIN\SSL no-shared
nmake
nmake install_sw

perl Configure VC-WIN64A --prefix=%CD%\..\BIN\Lib\x64\Release --openssldir=%CD%\..\BIN\SSL no-shared
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