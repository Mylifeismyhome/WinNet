title COMPILING CryptoPP

echo -------------------------------------------
echo Start compiling CryptoPP :: %date% %time%
echo -------------------------------------------
start /WAIT BUILDX86d.bat
start /WAIT BUILDX64d.bat
start /WAIT BUILDX86.bat
start /WAIT BUILDX64.bat
start /WAIT INSTALL.bat
echo -------------------------------------------
echo Done compiling CryptoPP :: %date% %time%
echo -------------------------------------------