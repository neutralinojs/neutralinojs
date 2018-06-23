/*
 * Author: Broglie 
 * E-mail: yibo141@outlook.com
 */

#include "Handler.h"
#include "router.h"

using namespace std;

Handler::Handler(const int connfd)
      :_connfd(connfd)
{
    _isClosed = false;
}

Handler::~Handler()
{
    if(!_isClosed)
        close(_connfd);
}

void Handler::handle()
{
    if(!receiveRequest())
    {
        close(_connfd);
        _isClosed = true;
        return;
    }
    pair<string, string> resp = routes::handle(_request.uri, _request.body);
    string content =  resp.first;
    std::string msg = "HTTP/1.1 200 OK\r\nContent-Type:" + resp.second + "\r\nContent-Length: " + std::to_string(content.size()) +"\r\nConnection: close\r\n\r\n" + content;
    _outputBuffer.append(msg.c_str(), msg.size());
    _outputBuffer.sendFd(_connfd);
    close(_connfd);
    _isClosed = true;
}

bool Handler::receiveRequest()
{ 
    if(_inputBuffer.readFd(_connfd) == 0)
        return false;
    std::string request = _inputBuffer.readAllAsString();
    Parser p(request);
    _request = p.getParseResult();
    return true;
}

void Handler::sendErrorMsg()
{

}

void Handler::parseURI()
{

}

void Handler::getFileType()
{

}