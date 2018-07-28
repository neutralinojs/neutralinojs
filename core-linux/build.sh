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
if [ -e bin/neutranino.out ]; then 
    rm bin/neutralino.out
fi

g++ -std=c++11 src/Buffer.cpp src/EventLoop.cpp src/EventLoopThreadPool.cpp src/Handler.cpp src/Parser.cpp src/Socket.cpp src/Epoll.cpp src/EventLoopThread.cpp src/functions.cpp src/main.cpp src/router.cpp src/Thread.cpp src/core/filesystem.cpp src/settings.cpp src/core/os.cpp src/core/computer.cpp src/auth/authbasic.cpp src/ping/ping.cpp src/core/storage/storage.cpp -pthread -std=c++14 -o bin/neutralino 