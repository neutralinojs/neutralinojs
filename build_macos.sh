echo "Compiling Neutralinojs..."

if [ -e bin/neutralino-mac ]; then
    rm bin/neutralino-mac
fi

c++ -arch x86_64 resources.cpp \
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
    platform/macos/platform.cpp \
    api/filesystem/filesystem.cpp \
    api/os/os.cpp \
    api/computer/computer.cpp \
    api/debug/debug.cpp \
    api/storage/storage.cpp \
    api/app/app.cpp \
    api/window/window.cpp \
    -I . \
    -std=c++17 \
    -pthread \
    -framework WebKit \
    -framework Cocoa \
    -DELPP_NO_DEFAULT_LOG_FILE=1 \
    -DWEBVIEW_COCOA=1 \
    -DOBJC_OLD_DISPATCH_PROTOTYPES=1 \
    -Os \
    -o bin/neutralino-mac 

if [ -e bin/neutralino-mac ]; then
    echo "OK: Neutralino binary is compiled in to bin/neutralino-mac"
else
    echo "ERR: Neutralino binary is not compiled"
fi
