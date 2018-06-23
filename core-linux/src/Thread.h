/*
 * Author: Broglie 
 * E-mail: yibo141@outlook.com
 */

#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <iostream>

class Thread 
{
public:
    typedef void* (*ThreadFunc)(void*);
    explicit Thread(ThreadFunc func, void *arg)
        :started(false),
         joined(false),
         _arg(arg),
         threadFunc(func)
    { }

    ~Thread()
    {
        if (started && !joined)
            pthread_detach(pthreadId);
    }

    void start();
    int join();
    bool isStarted() { return started; }
    pthread_t gettid() { return pthreadId; }

private:
    bool started;
    bool joined;
    void *_arg;
    pthread_t pthreadId;
    ThreadFunc threadFunc;
};

#endif // THREAD_H