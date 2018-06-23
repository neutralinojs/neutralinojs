/*
 * Author: Broglie 
 * E-mail: yibo141@outlook.com
 */

#ifndef CONDITION_H
#define CONDITION_H

#include <pthread.h>
#include "Mutex.h"

class Condition 
{
public:
    Condition(Mutex &m): mutex(m)
    {
        pthread_cond_init(&cond, NULL);
    }

    ~Condition() 
    {
        pthread_cond_destroy(&cond);
    }

    void wait() 
    {
        pthread_cond_wait(&cond, &mutex.getPthreadMutex());
    }

    void notify()
    {
        pthread_cond_signal(&cond);
    }

    void notifyAll()
    {
        pthread_cond_broadcast(&cond);
    }

private:
    Mutex mutex;
    pthread_cond_t cond;
};

#endif // CONDITION_H