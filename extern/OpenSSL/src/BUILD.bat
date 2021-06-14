echo -------------------------------------------
echo Start compiling OpenSSL :: %date% %time%
echo -------------------------------------------
start /WAIT INSTALL.bat
start /WAIT TEST.bat
start /WAIT BUILD_X64d_SHARED.bat
start /WAIT BUILD_X64_SHARED.bat
start /WAIT BUILD_X64d.bat
start /WAIT BUILD_X64.bat
start /WAIT BUILD_X86d_SHARED.bat
start /WAIT BUILD_X86_SHARED.bat
start /WAIT BUILD_X86d.bat
start /WAIT BUILD_X86.bat
start /WAIT "COPY.bat"
echo -------------------------------------------
echo Done compiling OpenSSL :: %date% %time%
echo -------------------------------------------