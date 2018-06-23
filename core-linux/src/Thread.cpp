/*
 * Author: Broglie 
 * E-mail: yibo141@outlook.com
 */

#include <pthread.h>
#include <assert.h>
#include "Thread.h"

void Thread::start()
{
    assert(!started);
    started = true;
    if(pthread_create(&pthreadId, NULL, threadFunc, _arg))
        started = false;
    //std::cout << "----------Thread created----------" << std::endl;
}

int Thread::join()
{
    assert(started && !joined);
    joined = true;
    return pthread_join(pthreadId, NULL);
}