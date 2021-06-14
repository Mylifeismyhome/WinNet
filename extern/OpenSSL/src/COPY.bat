REM Copy all needed files
xcopy %CD%\..\BIN\Lib\x64\Debug\lib\libcrypto.lib %CD%\..\..\lib\x64\Debug\ /F /R /Y /I /J >> openssl_copy.log
xcopy %CD%\..\BIN\Lib\x64\Debug\lib\libssl.lib %CD%\..\..\lib\x64\Debug\ /F /R /Y /I /J >> openssl_copy.log
xcopy %CD%\..\BIN\Lib\x64\Debug\lib\ossl_static.pdb %CD%\..\..\lib\x64\Debug\ /F /R /Y /I /J >> openssl_copy.log

xcopy %CD%\..\BIN\Lib\x64\Release\lib\libcrypto.lib %CD%\..\..\lib\x64\Release\ /F /R /Y /I /J >> openssl_copy.log
xcopy %CD%\..\BIN\Lib\x64\Release\lib\libssl.lib %CD%\..\..\lib\x64\Release\ /F /R /Y /I /J >> openssl_copy.log
xcopy %CD%\..\BIN\Lib\x64\Release\lib\ossl_static.pdb %CD%\..\..\lib\x64\Release\ /F /R /Y /I /J >> openssl_copy.log

xcopy %CD%\..\BIN\Lib\x64\Release\include\openssl\* %CD%\..\..\include\openssl\ /F /R /Y /I /J >> openssl_copy.log

exit