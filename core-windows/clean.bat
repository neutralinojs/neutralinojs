@echo off
echo Neutralino is cleaning...

if EXIST bin\neutralino.exe (
    del /f bin\neutralino.exe
)

if EXIST bin (
    rmdir bin
)

echo Neutralino binaries are cleaned!