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


#ifndef BUFFER_H
#define BUFFER_H

#include <string>
#include <vector>
#include <algorithm>
#include <unistd.h>

class Buffer 
{
public:
    Buffer(): _readIndex(0), _writeIndex(0) { }
    size_t readableBytes()
    {
        return _writeIndex - _readIndex;
    }
    size_t writableBytes()
    {
        return _buffer.size() - _writeIndex;
    }
    std::string readAllAsString()
    {
        std::string str(begin() + _readIndex, readableBytes());
        resetBuffer();
        return str;
    }
    void append(const char *data, const size_t len)
    {
        makeSpace(len);
        std::copy(data, data + len, beginWrite());
        _writeIndex += len;
    }

    char* peek() 
    {
        return static_cast<char*>(&*_buffer.begin()) + _readIndex;
    }

    size_t readFd(const int fd);
    void sendFd(const int fd);
    
private:
    const char* begin() const 
    {
        return &*_buffer.begin();
    }
    char* beginWrite()
    {
        return &*_buffer.begin() + _writeIndex;
    }
    void resetBuffer()
    {
        _readIndex = _writeIndex = 0;
        _buffer.clear();
        _buffer.shrink_to_fit();
    }
    void makeSpace(const size_t len)
    {
        if(writableBytes() < len)
            _buffer.resize(_writeIndex + len);
    }

    std::vector<char> _buffer;
    size_t _readIndex;
    size_t _writeIndex;
};

#endif // BUFFER_H
