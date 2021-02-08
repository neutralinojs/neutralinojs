# MIT License

# Copyright (c) 2018 Neutralinojs

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


echo "Neutralino is being built.."
if [ -e bin/neutralino ]; then
    rm bin/neutralino
fi

g++ -I ../core-shared -std=c++17 ../core-shared/log.cpp src/resources.cpp src/Buffer.cpp src/Handler.cpp src/requestparser.cpp src/Socket.cpp src/functions.cpp src/main.cpp src/router.cpp src/core/filesystem/filesystem.cpp src/settings.cpp src/core/os/os.cpp src/core/computer/computer.cpp src/core/debug/debug.cpp src/auth/authbasic.cpp src/ping/ping.cpp src/core/storage/storage.cpp src/core/app/app.cpp src/cloud/privileges.cpp -pthread -std=c++17 -DWEBVIEW_GTK=1 `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0` -o bin/neutralino -no-pie -lstdc++fs

if [ -e bin/neutralino ]; then
    echo "Neutralino binary is compiled in to bin/neutralino"
else
    echo "ERR : Neutralino binary is not compiled"
fi
