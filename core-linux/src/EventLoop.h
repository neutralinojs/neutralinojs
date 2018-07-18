// MIT License

// Copyright (c) 2018 Neutralinojs

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <sys/syscall.h>
#include <sys/types.h>
#include <assert.h>
#include <vector>
#include <poll.h>
#include <signal.h>
#include "Epoll.h"
#include "CurrentThread.h"

class Handler;
class EventLoop 
{
public:
    EventLoop();
    ~EventLoop();
    void loop();
    void quit();
    void addToLoop(const int fd);

private:
    void addToLoop();
    bool isLooping;
    const pid_t threadId;
    std::vector<int> fds;
    bool isQuit;
    Epoll *e;
};

#endif // EVENTLOOP_H