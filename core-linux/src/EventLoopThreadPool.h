/*
 * Author: Broglie 
 * E-mail: yibo141@outlook.com
 */

#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H

#include <vector>
#include "EventLoopThread.h"

class EventLoop;

class EventLoopThreadPool
{
public:
    EventLoopThreadPool(const int threadNum);
    ~EventLoopThreadPool();
    EventLoopThread* getNextThread();

private:
    int _threadNum;
    int _nextID;
    EventLoop *_baseLoop;
    std::vector<EventLoopThread*> _threads;
};

#endif // EVENTLOOPTREADPOOL_H