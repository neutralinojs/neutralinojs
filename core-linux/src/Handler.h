/*
 * Author: Broglie 
 * E-mail: yibo141@outlook.com
 */

#ifndef HANDLER_H
#define HANDLER_H

#include <string>
#include <algorithm>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "Parser.h"
#include "Buffer.h"

class Handler 
{
public:
    Handler(const int connfd);
    ~Handler();
    void handle();
    const int connFd() const
    {
        return _connfd;
    }
private:
    bool receiveRequest();  // 接受客户的请求并解析
    void sendResponse();    // 发送相应
    void sendErrorMsg();
    void parseURI();
    void getFileType();
    int _connfd;
    bool _isClosed;
    std::string _fileType;
    std::string _fileName;
    Buffer _inputBuffer;
    Buffer _outputBuffer;
    HTTPRequest _request;
};

#endif // HANDLER_H