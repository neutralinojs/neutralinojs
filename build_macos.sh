ARCH=$1
# Trigger x64 build by default
if [ "$ARCH" == "" ]; then
    ARCH="x64"
fi

NEU_BIN="./bin/neutralino-mac_$ARCH"

if [ "$ARCH" == "ia32" ]; then
    FLAGS="-arch i386"
elif [ "$ARCH" == "x64" ]; then
    FLAGS="-arch x86_64"
else
    echo "Unsupported instruction set architecture: $ARCH"
    exit 1
fi

if [ -e $NEU_BIN ]; then
    rm $NEU_BIN
fi

echo "Compiling Neutralinojs $ARCH..."

c++ $FLAGS resources.cpp \
    server/router.cpp \
    server/ping.cpp \
    server/neuserver.cpp \
    main.cpp \
    helpers.cpp \
    settings.cpp \
    auth/authbasic.cpp \
    auth/permission.cpp \
    lib/boxer/boxer_osx.mm \
    lib/easylogging/easylogging++.cc \
    api/filesystem/filesystem.cpp \
    api/os/os.cpp \
    api/computer/computer.cpp \
    api/debug/debug.cpp \
    api/storage/storage.cpp \
    api/app/app.cpp \
    api/window/window.cpp \
    api/events/events.cpp \
    -I . \
    -std=c++17 \
    -pthread \
    -framework WebKit \
    -framework Cocoa \
    -DELPP_NO_DEFAULT_LOG_FILE=1 \
    -DWEBVIEW_COCOA=1 \
    -DOBJC_OLD_DISPATCH_PROTOTYPES=1 \
    -Os \
    -o $NEU_BIN 

if [ -e $NEU_BIN ]; then
    echo "OK: Neutralino binary is compiled into $NEU_BIN"
else
    echo "ERR: Neutralino binary is not compiled"
    exit 1
fi
