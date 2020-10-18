@echo off

for /L %%I in (1,1,100) do (
  start Client.exe
)

pause