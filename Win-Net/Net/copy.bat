@echo OFF
echo ---------- NET XCOPY INCLUDE FILES ----------
xcopy "..\..\..\extern\crypto++\include\" "..\..\extern\Net\include\" /s /i /q /y /c
xcopy "..\..\..\extern\JSON\" "..\..\extern\Net\include\JSON\" /s /i /q /y /c
xcopy "..\..\..\extern\MYSQL\include\" "..\..\extern\Net\include\MYSQL\" /s /i /q /y /c

REM Copy from any source available, all include headers are the same
xcopy "..\..\..\extern\OpenSSL\bin\dll\x64\release\include" "..\..\extern\Net\include\" /s /i /q /y /c /d
xcopy "..\..\..\extern\OpenSSL\bin\dll\x64\debug\include" "..\..\extern\Net\include\" /s /i /q /y /c /d
xcopy "..\..\..\extern\OpenSSL\bin\dll\x86\release\include" "..\..\extern\Net\include\" /s /i /q /y /c /d
xcopy "..\..\..\extern\OpenSSL\bin\dll\x86\debug\include" "..\..\extern\Net\include\" /s /i /q /y /c /d
xcopy "..\..\..\extern\OpenSSL\bin\lib\x64\release\include" "..\..\extern\Net\include\" /s /i /q /y /c /d
xcopy "..\..\..\extern\OpenSSL\bin\lib\x86\release\include" "..\..\extern\Net\include\" /s /i /q /y /c /d
xcopy "..\..\..\extern\OpenSSL\bin\lib\x64\debug\include" "..\..\extern\Net\include\" /s /i /q /y /c /d
xcopy "..\..\..\extern\OpenSSL\bin\lib\x86\debug\include" "..\..\extern\Net\include\" /s /i /q /y /c /d

xcopy "..\..\..\extern\pugiXML\src\*.hpp" "..\..\extern\Net\include\pugiXML\" /s /i /q /y /c
xcopy "..\..\..\extern\ZLib\*.h" "..\..\extern\Net\include\ZLib\" /s /i /q /y /c
xcopy "..\..\..\extern\BCrypt\" "..\..\extern\Net\include\BCrypt\" /s /i /q /y /c
xcopy "..\..\Net\*.h" "..\..\extern\Net\include\" /s /i /q /y /c
xcopy "..\..\Net\*.hpp" "..\..\extern\Net\include\" /s /i /q /y /c
echo ---------- NET XCOPY INCLUDE FILES FINISHED ----------

echo ---------- NET XCOPY LIB FILES ----------
xcopy "..\..\bin\%1.lib" "..\..\extern\Net\lib\%2" /s /i /q /y /c
echo ---------- NET XCOPY FINISHED ----------