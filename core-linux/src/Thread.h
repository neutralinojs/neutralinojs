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