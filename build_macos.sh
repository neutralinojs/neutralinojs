#!/bin/sh

ARCH=$1
# Trigger x64 build by default
if [ "$ARCH" = "" ]; then
    ARCH="x64"
fi

NEU_BIN="./bin/neutralino-mac_$ARCH"

if [ "$ARCH" = "ia32" ]; then
    FLAGS="-arch i386"
elif [ "$ARCH" = "x64" ]; then
    FLAGS="-arch x86_64"
elif [ "$ARCH" = "arm64" ]; then
    FLAGS="-arch arm64"
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
    server/neuserver.cpp \
    main.cpp \
    helpers.cpp \
    settings.cpp \
    extensions_loader.cpp \
    chrome.cpp \
    auth/authbasic.cpp \
    auth/permission.cpp \
    lib/tinyprocess/process.cpp \
    lib/tinyprocess/process_unix.cpp \
    lib/easylogging/easylogging++.cc \
    lib/platformfolders/platform_folders.cpp \
    lib/clip/clip.cpp \
    lib/clip/image.cpp \
    lib/clip/clip_osx.mm \
    api/filesystem/filesystem.cpp \
    api/os/os.cpp \
    api/computer/computer.cpp \
    api/debug/debug.cpp \
    api/storage/storage.cpp \
    api/app/app.cpp \
    api/window/window.cpp \
    api/events/events.cpp \
    api/extensions/extensions.cpp \
    api/clipboard/clipboard.cpp \
    -I . \
    -I lib/asio/include \
    -I lib \
    -L lib \
    -std=c++17 \
    -pthread \
    -framework WebKit \
    -framework Cocoa \
    -DELPP_NO_DEFAULT_LOG_FILE=1 \
    -DASIO_STANDALONE \
    -DWEBVIEW_COCOA=1 \
    -DTRAY_APPKIT=1 \
    -DOBJC_OLD_DISPATCH_PROTOTYPES=1 \
    -Wno-deprecated-declarations \
    -Os \
    -o $NEU_BIN

if [ -e $NEU_BIN ]; then
    echo "OK: Neutralino binary is compiled into $NEU_BIN"
else
    echo "ERR: Neutralino binary is not compiled"
    exit 1
fi
