echo "Compiling Neutralinojs..."

if [ -e bin/neutralino ]; then
    rm bin/neutralino
fi

c++ -arch x86_64 ../core-shared/resources.cpp \
    ../core-shared/router.cpp \
    ../core-shared/permission.cpp \
    ../core-shared/main.cpp \
    ../core-shared/helpers.cpp \
    ../core-shared/settings.cpp \
    ../core-shared/auth/authbasic.cpp \
    ../core-shared/ping/ping.cpp \
    ../core-shared/server/neuserver.cpp \
    ../core-shared/lib/boxer/boxer_osx.mm \
    ../core-shared/lib/easylogging/easylogging++.cc \
    src/platform/macos.cpp \
    src/api/filesystem/filesystem.cpp \
    src/api/os/os.cpp \
    src/api/computer/computer.cpp \
    src/api/debug/debug.cpp \
    src/api/storage/storage.cpp \
    src/api/app/app.cpp \
    src/api/window/window.cpp \
    -I ../core-shared \
    -std=c++17 \
    -pthread \
    -framework WebKit \
    -framework Cocoa \
    -DELPP_NO_DEFAULT_LOG_FILE=1 \
    -DWEBVIEW_COCOA=1 \
    -Os \
    -o bin/neutralino 

if [ -e bin/neutralino ]; then
    echo "OK: Neutralino binary is compiled in to bin/neutralino"
else
    echo "ERR: Neutralino binary is not compiled"
fi
