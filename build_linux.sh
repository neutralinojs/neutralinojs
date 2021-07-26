ARCH=$1
# Trigger x64 build by default
if [ "$ARCH" == "" ]; then
    ARCH="x64"
fi

NEU_BIN="neutralino-linux_$ARCH"

if [ "$ARCH" == "ia32" ]; then
    export PKG_CONFIG_PATH=/usr/lib/i386-linux-gnu/pkgconfig
    FLAGS="-m32"
elif [ "$ARCH" == "x64" ]; then
    export PKG_CONFIG_PATH=/usr/lib/pkgconfig
else
    echo "Unsupported instruction set architecture: $ARCH"
    exit 1
fi

if [ -e bin/$NEU_BIN ]; then
    rm bin/$NEU_BIN
fi

echo "Compiling Neutralinojs $ARCH..."

g++ $FLAGS resources.cpp \
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
    `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0 glib-2.0 appindicator3-0.1` \
    -o bin/$NEU_BIN \
    -no-pie \
    -Os \
     -I .

if [ -e bin/$NEU_BIN ]; then
    echo "OK: Neutralino binary is compiled in to bin/$NEU_BIN"
else
    echo "ERR: Neutralino binary is not compiled"
    exit 1
fi
