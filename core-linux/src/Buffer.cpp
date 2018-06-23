/*
 * Author: Broglie 
 * E-mail: yibo141@outlook.com
 */

#include "Buffer.h"

size_t Buffer::readFd(const int fd)
{
    char extrabuf[65535];
    char *ptr = extrabuf;
    int nleft = 65535;
    int nread;
    while((nread = read(fd, ptr, nleft)) < 0)
    {
        if(errno == EINTR)
            nread = 0;
        else 
            return 0;
    }
    append(extrabuf, nread);
    return nread;
}

void Buffer::sendFd(const int fd)
{
    size_t bytesSent = 0;
    size_t bytesLeft = readableBytes();
    char *ptr = peek();
    while(bytesLeft)
    {
        if((bytesSent = write(fd, ptr, bytesLeft)) < 0)
        {
            if(bytesSent < 0 && errno == EINTR)
                bytesSent = 0;
            else
                return;
        }
        bytesLeft -= bytesSent;
        ptr += bytesSent;
    } 
    resetBuffer();
}