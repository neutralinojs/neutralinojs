@echo off
echo Neutralino is being built...
if NOT EXIST bin (
    mkdir bin
)
g++ src/main.cpp src/settings.cpp src/requestparser.cpp src/serverlistener.cpp src/functions.cpp src/core/computer.cpp src/core/filesystem.cpp src/core/os.cpp src/router.cpp -lws2_32 -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic -o bin/neutralino 

echo Neutralino binary is compiled in to bin/netralino.exe


