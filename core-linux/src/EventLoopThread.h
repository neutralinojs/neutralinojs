/*
 * Author: Broglie 
 * E-mail: yibo141@outlook.com
 */

#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H

#include <functional>
#include "Thread.h"
#include "EventLoop.h"

class EventLoopThread
{
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* getLoop()
    {
        return _loop;
    }

private:
    static void* threadFunc(void *arg);
    EventLoop *_loop;
    Thread _thread;
};

#endif // EVENTLOOPTHREAD_H