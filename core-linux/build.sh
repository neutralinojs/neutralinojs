echo "Compiling Neutralinojs..."
if [ -e bin/neutralino ]; then
    rm bin/neutralino
fi

g++ ../core-shared/resources.cpp \
    ../core-shared/helpers.cpp \
    ../core-shared/main.cpp \
    ../core-shared/router.cpp \
    ../core-shared/settings.cpp \
    ../core-shared/auth/authbasic.cpp \
    ../core-shared/ping/ping.cpp \
    ../core-shared/permission.cpp \
    ../core-shared/lib/boxer/boxer_linux.cpp \
    ../core-shared/lib/easylogging/easylogging++.cc \
    ../core-shared/server/neuserver.cpp \
    src/platform/linux.cpp \
    src/api/filesystem/filesystem.cpp \
    src/api/os/os.cpp \
    src/api/computer/computer.cpp \
    src/api/debug/debug.cpp \
    src/api/storage/storage.cpp \
    src/api/app/app.cpp \
    src/api/window/window.cpp \
    -pthread \
    -std=c++17 \
    -DELPP_NO_DEFAULT_LOG_FILE=1 \
    -DWEBVIEW_GTK=1 \
    `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0 glib-2.0` \
    -o bin/neutralino \
    -no-pie \
     -I ../core-shared

if [ -e bin/neutralino ]; then
    echo "OK: Neutralino binary is compiled in to bin/neutralino"
else
    echo "ERR: Neutralino binary is not compiled"
fi
