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

#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include "Handler.h"

class EventLoop;

class Epoll 
{
public:
    explicit Epoll(EventLoop *loop)
        :ownerLoop(loop),
         epollfd(epoll_create1(EPOLL_CLOEXEC)),
         retEvents(8) // 初始时为epoll_wait预留8个返回的epoll_event
    { 
        // FIXME: 调用日志库写入日志并终止程序
        if(epollfd < 0)
            std::cout << "Epoll::epoll_create1() error: " << ::strerror(errno) << std::endl;
        assert(epollfd > 0);
    }

    ~Epoll()
    { close(epollfd); }

    // 调用epoll_wait，并将其交给Event类的handleEvent函数处理
    void epoll(std::vector<Handler*> &events);
    void removeFd(const int fd);
    /*
    void assertInLoopThread() const 
    {
        ownerLoop->assertInLoopThread();
    }
    */
    void addToEpoll(const int fd);

private: 
    EventLoop *ownerLoop;
    int epollfd;
    std::vector<struct epoll_event> retEvents;
    // std::vector<Event> events;
};

#endif // EPOLL_H