@echo off

cd core-windows
cmd /c build.bat
cd ..
xcopy /s /y core-windows\bin dist



