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
