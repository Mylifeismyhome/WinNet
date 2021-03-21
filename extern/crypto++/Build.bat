echo -------------------------------------------
echo Start compiling CryptoPP :: %date% %time%
echo -------------------------------------------
call BUILDX86d.bat
call BUILDX64d.bat
call BUILDX86.bat
call BUILDX64.bat
echo -------------------------------------------
echo Done compiling CryptoPP :: %date% %time%
echo -------------------------------------------