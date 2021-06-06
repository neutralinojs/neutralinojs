echo "Compiling Neutralinojs..."
if [ -e bin/neutralino-linux ]; then
    rm bin/neutralino-linux
fi

g++ resources.cpp \
    helpers.cpp \
    main.cpp \
    server/router.cpp \
    server/neuserver.cpp \
    server/ping.cpp \
    settings.cpp \
    auth/authbasic.cpp \
    auth/permission.cpp \
    lib/boxer/boxer_linux.cpp \
    lib/easylogging/easylogging++.cc \
    platform/linux/platform.cpp \
    api/filesystem/filesystem.cpp \
    api/os/os.cpp \
    api/computer/computer.cpp \
    api/debug/debug.cpp \
    api/storage/storage.cpp \
    api/app/app.cpp \
    api/window/window.cpp \
    -pthread \
    -std=c++17 \
    -DELPP_NO_DEFAULT_LOG_FILE=1 \
    -DWEBVIEW_GTK=1 \
    `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0 glib-2.0` \
    -o bin/neutralino-linux \
    -no-pie \
     -I .

if [ -e bin/neutralino-linux ]; then
    echo "OK: Neutralino binary is compiled in to bin/neutralino-linux"
else
    echo "ERR: Neutralino binary is not compiled"
fi
