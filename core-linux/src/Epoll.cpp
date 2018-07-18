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

#include "Epoll.h"

void Epoll::epoll(std::vector<Handler*> &activeEvents)
{
    int numEvents = epoll_wait(epollfd,
                                 retEvents.data(),
                                 static_cast<int>(retEvents.size()),
                                 1000);
    if(numEvents < 0) 
    {
        if(errno != EINTR)
        {   
            std::cout << "Epoll::epoll() error: " << strerror(errno) << std::endl;
            exit(1);
        }
    }
    else if(numEvents == 0) 
    {

    }
    else 
    {
        /*
        if(numEvents == retEvents.size())
            retEvents.resize(1.5 * numEvents);
        */
        for(int i = 0; i < numEvents; ++i) 
        {
            Handler *h = new Handler(retEvents[i].data.fd);
            activeEvents.push_back(h);
        }
    }
}

void Epoll::removeFd(const int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
}

void Epoll::addToEpoll(const int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLPRI | EPOLLRDHUP;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event) < 0)
        std::cout << "Epoll::addToEpoll error: " << strerror(errno) << std::endl;
}