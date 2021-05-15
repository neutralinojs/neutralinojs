echo "Neutralino is being built.."
if [ -e bin/neutralino ]; then
    rm bin/neutralino
fi

g++ -I ../core-shared -std=c++17 ../core-shared/lib/easylogging/easylogging++.cc ../core-shared/resources.cpp src/server/serverlistener.cpp src/server/Buffer.cpp src/server/Handler.cpp src/server/requestparser.cpp src/server/Socket.cpp ../core-shared/helpers.cpp src/main.cpp ../core-shared/router.cpp src/api/filesystem/filesystem.cpp src/platform/linux.cpp ../core-shared/settings.cpp src/api/os/os.cpp src/api/computer/computer.cpp src/api/debug/debug.cpp ../core-shared/auth/authbasic.cpp ../core-shared/ping/ping.cpp src/api/storage/storage.cpp src/api/app/app.cpp src/api/window/window.cpp ../core-shared/permission.cpp -pthread -std=c++17 -DELPP_NO_DEFAULT_LOG_FILE=1 -DWEBVIEW_GTK=1 `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0` -o bin/neutralino -no-pie -lstdc++fs

if [ -e bin/neutralino ]; then
    echo "Neutralino binary is compiled in to bin/neutralino"
else
    echo "ERR : Neutralino binary is not compiled"
fi
