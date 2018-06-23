/*
 * Author: Broglie 
 * E-mail: yibo141@outlook.com
 */

#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(const int threadNum)
    : _threadNum(threadNum),
      _nextID(0)
{
    for(int i = 0; i < threadNum; ++i)
    {
        EventLoopThread *thread = new EventLoopThread();
        _threads.push_back(thread);
    }
}

EventLoopThreadPool::~EventLoopThreadPool()
{
    for(int i = 0; i < _threads.size(); ++i)
        delete _threads[i];
}

// 每次以轮转的方式取一个工作线程
EventLoopThread* EventLoopThreadPool::getNextThread()
{
    if(_nextID >= _threadNum)
        _nextID = 0;
    return _threads[_nextID++];
}