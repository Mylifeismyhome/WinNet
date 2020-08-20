cd openssl-master

nmake clean
perl Configure VC-WIN64A --debug --prefix=C:\Rooky\extern\OpenSSL\src\BIN\DLL\x64\Debug --openssldir=C:\Rooky\extern\OpenSSL\src\BIN\SSL
nmake
nmake install_sw

perl Configure VC-WIN64A --prefix=C:\Rooky\extern\OpenSSL\src\BIN\DLL\x64\Release --openssldir=C:\Rooky\extern\OpenSSL\src\BIN\SSL
nmake
nmake install_sw

perl Configure VC-WIN64A --debug --prefix=C:\Rooky\extern\OpenSSL\src\BIN\Lib\x64\Debug --openssldir=C:\Rooky\extern\OpenSSL\src\BIN\SSL no-shared
nmake
nmake install_sw

perl Configure VC-WIN64A --prefix=C:\Rooky\extern\OpenSSL\src\BIN\Lib\x64\Release --openssldir=C:\Rooky\extern\OpenSSL\src\BIN\SSL no-shared
nmake
nmake install_sw

REM Copy all needed files
xcopy BIN\Lib\x64\Debug\lib\libcrypto.lib ..\lib\x64\Debug\ /F /R /Y /I /J >> openssl_copy.log
xcopy BIN\Lib\x64\Debug\lib\libssl.lib ..\lib\x64\Debug\ /F /R /Y /I /J >> openssl_copy.log
xcopy BIN\Lib\x64\Debug\lib\ossl_static.pdb ..\lib\x64\Debug\ /F /R /Y /I /J >> openssl_copy.log

xcopy BIN\Lib\x64\Release\lib\libcrypto.lib ..\lib\x64\Release\ /F /R /Y /I /J >> openssl_copy.log
xcopy BIN\Lib\x64\Release\lib\libssl.lib ..\lib\x64\Release\ /F /R /Y /I /J >> openssl_copy.log
xcopy BIN\Lib\x64\Release\lib\ossl_static.pdb ..\lib\x64\Release\ /F /R /Y /I /J >> openssl_copy.log

xcopy BIN\Lib\x64\Release\include\openssl\* ..\include\openssl\ /F /R /Y /I /J >> openssl_copy.log