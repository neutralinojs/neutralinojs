/*
 * Author: Broglie 
 * E-mail: yibo141@outlook.com
 */

#ifndef CURRENTTHREAD_H
#define CURRENTTHREAD_H

#include <sys/syscall.h>
#include <unistd.h>
#include <sys/types.h>

class CurrentThread 
{
public:
    static pid_t gettid()
    {
        return static_cast<pid_t>(syscall(SYS_gettid));
    }
};

#endif // CURRENTTHREAD_H