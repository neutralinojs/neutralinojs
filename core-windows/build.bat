@echo off

REM MIT License

REM Copyright (c) 2018 Neutralinojs

REM Permission is hereby granted, free of charge, to any person obtaining a copy
REM of this software and associated documentation files (the "Software"), to deal
REM in the Software without restriction, including without limitation the rights
REM to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
REM copies of the Software, and to permit persons to whom the Software is
REM furnished to do so, subject to the following conditions:

REM The above copyright notice and this permission notice shall be included in all
REM copies or substantial portions of the Software.

REM THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
REM IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
REM FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
REM AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
REM LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
REM OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
REM SOFTWARE.

@echo off
echo Neutralino is being built...

if EXIST bin\neutralino.exe (
    del /f bin\neutralino.exe 
)

g++  -std=gnu++17 -lstdc++fs -fconcepts -I ../core-shared ../core-shared/log.cpp src/main.cpp ../core-shared/settings.cpp ../core-shared/resources.cpp src/server/requestparser.cpp src/server/serverlistener.cpp ../core-shared/helpers.cpp src/api/computer/computer.cpp src/api/filesystem/filesystem.cpp src/api/os/os.cpp ../core-shared/router.cpp ../core-shared/auth/authbasic.cpp ../core-shared/ping/ping.cpp src/api/storage/storage.cpp src/api/debug/debug.cpp src/api/app/app.cpp ../core-shared/cloud/privileges.cpp src/platform/windows.cpp ../core-shared/webview/webview_c.cpp -lws2_32 -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic -mwindows -Wconversion-null -DWEBVIEW_WINAPI=1 -lole32 -lcomctl32 -loleaut32 -luuid -lgdiplus -mwindows -lshlwapi -o bin/neutralino

if EXIST bin\neutralino.exe (
    echo Neutralino binary is compiled in to bin/netralino.exe
)

if NOT EXIST bin\neutralino.exe (
    echo ERR : Neutralino binary is not compiled
)

