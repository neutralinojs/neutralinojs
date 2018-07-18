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

#include "EventLoop.h"

class IgnoreSigPipe
{
public:
    IgnoreSigPipe()
    {
        signal(SIGPIPE, SIG_IGN);
    }
};

// 忽略SIGPIPE信号
IgnoreSigPipe initObj;

EventLoop::EventLoop()
    :isLooping(false),
     threadId(CurrentThread::gettid()),
     isQuit(false),
     e(new Epoll(this))
{ }

EventLoop::~EventLoop()
{
    assert(!isLooping);
}

void EventLoop::loop()
{
    assert(!isLooping);
    isLooping = true;
    isQuit = false;

    while(!isQuit)
    {
        //std::cout << "----------Looping----------" << std::endl;
        addToLoop();
        std::vector<Handler*> activeEvents;
        activeEvents.clear();
        // 调用epoll将活动的套接字描述符对应的Handler取出并处理
        e->epoll(activeEvents);
        for(std::vector<Handler*>::iterator iter = activeEvents.begin();
            iter != activeEvents.end(); ++iter)
        {
            //std::cout << "----------Handle request----------" << std::endl;
            // 处理客户端请求的入口
            (*iter)->handle();
            e->removeFd((*iter)->connFd());
            delete *iter;
        }
    }
    isLooping = false;
}

void EventLoop::quit()
{
    isQuit = true;
}

// 每次将一个套接字描述符添加到描述符数组中
void EventLoop::addToLoop(const int fd)
{
    // e->addToEpoll(fd);
    //std::cout << "----------Add " << fd << " to loop----------" << std::endl;
    fds.push_back(fd);
}

// 将描述符数组中的所有套接字全部添加到epoll中
void EventLoop::addToLoop()
{
    if(fds.empty())
    {
        //std::cout << "----------fds empty----------" << std::endl;
        return;
    }
    for(int i = 0; i < fds.size(); ++i)
        e->addToEpoll(fds[i]);
    fds.clear();
    //std::cout << "----------Add all fd to loop----------" << std::endl;
}