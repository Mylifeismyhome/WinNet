cd openssl-master

nmake clean
perl Configure VC-WIN32 --debug --prefix=C:\Rooky\extern\OpenSSL\src\BIN\DLL\x86\Debug --openssldir=C:\Rooky\extern\OpenSSL\src\BIN\SSL
nmake
nmake install_sw

perl Configure VC-WIN32 --prefix=C:\Rooky\extern\OpenSSL\src\BIN\DLL\x86\Release --openssldir=C:\Rooky\extern\OpenSSL\src\BIN\SSL
nmake
nmake install_sw

perl Configure VC-WIN32 --debug --prefix=C:\Rooky\extern\OpenSSL\src\BIN\Lib\x86\Debug --openssldir=C:\Rooky\extern\OpenSSL\src\BIN\SSL no-shared
nmake
nmake install_sw

perl Configure VC-WIN32 --prefix=C:\Rooky\extern\OpenSSL\src\BIN\Lib\x86\Release --openssldir=C:\Rooky\extern\OpenSSL\src\BIN\SSL no-shared
nmake
nmake install_sw

REM Copy all needed files
xcopy BIN\Lib\x86\Debug\lib\libcrypto.lib ..\lib\x86\Debug\ /F /R /Y /I /J >> openssl_copy.log
xcopy BIN\Lib\x86\Debug\lib\libssl.lib ..\lib\x86\Debug\ /F /R /Y /I /J >> openssl_copy.log
xcopy BIN\Lib\x86\Debug\lib\ossl_static.pdb ..\lib\x86\Debug\ /F /R /Y /I /J >> openssl_copy.log

xcopy BIN\Lib\x86\Release\lib\libcrypto.lib ..\lib\x86\Release\ /F /R /Y /I /J >> openssl_copy.log
xcopy BIN\Lib\x86\Release\lib\libssl.lib ..\lib\x86\Release\ /F /R /Y /I /J >> openssl_copy.log
xcopy BIN\Lib\x86\Release\lib\ossl_static.pdb ..\lib\x86\Release\ /F /R /Y /I /J >> openssl_copy.log

xcopy BIN\Lib\x86\Release\include\openssl\* ..\include\openssl\ /F /R /Y /I /J >> openssl_copy.log