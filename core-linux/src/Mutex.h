/*
 * Author: Broglie 
 * E-mail: yibo141@outlook.com
 */

#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>

class Mutex
{
public:
    Mutex() 
    {
        pthread_mutex_init(&mutex, NULL);
    }

    ~Mutex()
    {
        pthread_mutex_destroy(&mutex);
    }

    void lock()
    {
        pthread_mutex_lock(&mutex);
    }

    void unlock()
    {
        pthread_mutex_unlock(&mutex);
    }

    pthread_mutex_t& getPthreadMutex()
    {
        return mutex;
    }

private:
    pthread_mutex_t mutex;
};

class MutexLock
{
public:
    explicit MutexLock(Mutex &m): mutex(m)
    {
        mutex.lock();
    }

    ~MutexLock()
    {
        mutex.unlock();
    }

private:
    Mutex mutex;
};

#endif // MUTEX_H