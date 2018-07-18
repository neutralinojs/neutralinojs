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

#include "Socket.h"

void Socket::setReuseAddr(const int fd, bool on)
{
    int optval = on ? 1 : 0;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                     &optval, sizeof(optval));
}

void Socket::setTcpNoDelay(const int fd, bool on)
{
    int optval = on ? 1 : 0;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

int Socket::createSocket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd < 0)
    {
        std::cout << "Socket::createNonblock error: " << strerror(errno) << std::endl;
        exit(1);
    }
    return sockfd;  
}

void Socket::Bind(const int sockfd, const struct sockaddr_in &addr)
{
    int ret = bind(sockfd, (struct sockaddr*)(&addr), sizeof(addr));
    if(ret < 0) 
    {
        std::cout << "Socket::bind error: " << strerror(errno) << std::endl;
        exit(1);
    }
}

void Socket::Listen(const int sockfd)
{
    if(listen(sockfd, 5) < 0)
    {
        std::cout << "Socket::listen error: " << strerror(errno) << std::endl;
        exit(1);
    }
}

int Socket::Accept(const int sockfd, struct sockaddr_in *addr)
{
    socklen_t addrLen = sizeof(*addr);
    int connfd = accept4(sockfd, (struct sockaddr*)&addr, 
                         &addrLen, SOCK_NONBLOCK | SOCK_CLOEXEC);

    if(connfd < 0)
    {
        switch(errno)
        {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EMFILE:
                break;
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOMEM:
                std::cout << "Socket::accept error: " << strerror(errno) << std::endl;
                exit(1);
                break;
            default:
                std::cout << "Socket::accept error: " << strerror(errno) << std::endl;
                exit(1);
                break;
        }
    }
    return connfd;
}

void Socket::Close(const int sockfd)
{
    if(close(sockfd) < 0)
    {
        std::cout << "Socket::close error: " << strerror(errno) << std::endl;
        exit(1);
    }
}

void Socket::setNonBlockAndCloseOnExec(const int sockfd)
{

}