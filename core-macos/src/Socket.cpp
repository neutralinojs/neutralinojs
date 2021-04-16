 /*
 * Author: Broglie
 * E-mail: yibo141@outlook.com
 */

#include "Socket.h"
#include "core/include/log.h"

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
        ERROR() << "Socket::createNonblock error: " << strerror(errno);
        exit(1);
    }
    return sockfd;
}

void Socket::Bind(const int sockfd, const struct sockaddr_in &addr)
{
    int ret = bind(sockfd, (struct sockaddr*)(&addr), sizeof(addr));
    if(ret < 0)
    {
        ERROR() << "Socket::bind error: " << strerror(errno);
        exit(1);
    }
}

void Socket::Listen(const int sockfd)
{
    if(listen(sockfd, 5) < 0)
    {
        ERROR() << "Socket::listen error: " << strerror(errno);
        exit(1);
    }
}

int Socket::Accept(const int sockfd, struct sockaddr_in *addr)
{
    socklen_t addrLen = sizeof(*addr);
    int connfd = accept(sockfd, NULL , NULL);

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
                ERROR() << "Socket::accept error: " << strerror(errno);
                exit(1);
                break;
            default:
                ERROR() << "Socket::accept error: " << strerror(errno);
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
        ERROR() << "Socket::close error: " << strerror(errno);
        exit(1);
    }
}

void Socket::setNonBlockAndCloseOnExec(const int sockfd)
{

}
