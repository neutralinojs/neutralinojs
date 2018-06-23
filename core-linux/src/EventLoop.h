/*
 * Author: Broglie 
 * E-mail: yibo141@outlook.com
 */

#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <sys/syscall.h>
#include <sys/types.h>
#include <assert.h>
#include <vector>
#include <poll.h>
#include <signal.h>
#include "Epoll.h"
#include "CurrentThread.h"

class Handler;
class EventLoop 
{
public:
    EventLoop();
    ~EventLoop();
    void loop();
    void quit();
    void addToLoop(const int fd);

private:
    void addToLoop();
    bool isLooping;
    const pid_t threadId;
    std::vector<int> fds;
    bool isQuit;
    Epoll *e;
};

#endif // EVENTLOOP_H