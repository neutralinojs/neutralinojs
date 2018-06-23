/*
 * Author: Broglie 
 * E-mail: yibo141@outlook.com
 */

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