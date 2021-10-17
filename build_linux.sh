ARCH=$1
# Trigger x64 build by default
if [ "$ARCH" == "" ]; then
    ARCH="x64"
fi

NEU_BIN="./bin/neutralino-linux_$ARCH"

if [ -e $NEU_BIN ]; then
    rm $NEU_BIN
fi

echo "Compiling Neutralinojs $ARCH..."

g++ resources.cpp \
    helpers.cpp \
    main.cpp \
    server/router.cpp \
    server/neuserver.cpp \
    server/ping.cpp \
    settings.cpp \
    auth/authbasic.cpp \
    auth/permission.cpp \
    lib/easylogging/easylogging++.cc \
    lib/platformfolders/platform_folders.cpp \
    api/filesystem/filesystem.cpp \
    api/os/os.cpp \
    api/computer/computer.cpp \
    api/debug/debug.cpp \
    api/storage/storage.cpp \
    api/app/app.cpp \
    api/window/window.cpp \
    api/events/events.cpp \
    -pthread \
    -std=c++17 \
    -DELPP_NO_DEFAULT_LOG_FILE=1 \
    -DBOOST_ALL_NO_LIB \
    -DWEBVIEW_GTK=1 \
    `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0 glib-2.0 appindicator3-0.1` \
    -o $NEU_BIN \
    -no-pie \
    -Os \
    -I . \
    -I lib \
    -L lib

if [ -e $NEU_BIN ]; then
    echo "OK: Neutralino binary is compiled into $NEU_BIN"
else
    echo "ERR: Neutralino binary is not compiled"
    exit 1
fi
