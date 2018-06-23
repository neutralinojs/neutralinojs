/*
 * Author: Broglie 
 * E-mail: yibo141@outlook.com
 */

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
        // FIXME: 写入日志
        std::cout << "Socket::createNonblock error: " << strerror(errno) << std::endl;
        exit(1);
    }
    // setNonBlockAndCloseOnExec(sockfd);
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
    // int connfd = accept(sockfd, (struct sockaddr*)&addr, &addrLen);
    // setNonBlockAndCloseOnExec(connfd);
    int connfd = accept4(sockfd, (struct sockaddr*)&addr, 
                         &addrLen, SOCK_NONBLOCK | SOCK_CLOEXEC);

    if(connfd < 0)
    {
        switch(errno)
        {
            // 非致命错误，忽略
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EMFILE:
                break;
            // 致命错误，退出程序
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOMEM:
                std::cout << "Socket::accept error: " << strerror(errno) << std::endl;
                exit(1);
                break;
            // 未知错误，退出程序
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
    // 这段代码有问题，一直无法成功执行(Bad file descriptor)，并且
    // fcntl函数返回-1，不知道为什么。所以用到这个函数的地方就用别的方法
    // 替代了，如socket和accpet4。
    /*
    int flags;
    if((flags = fcntl(sockfd, F_GETFL, 0)) < 0)
    {
        std::cout << "Socket::setNonBlockAndCloseOnExec error: " <<
                    strerror(errno) << std::endl;
        exit(1);
    }
    flags |= O_NONBLOCK;
    flags |= FD_CLOEXEC;
    if(fcntl(sockfd, F_SETFL, flags) < 0)
    {
        std::cout << "Socket::setNonBlockAndCloseOnExec error: " <<
                    strerror(errno) << std::endl;
        exit(1);
    }
    */
}