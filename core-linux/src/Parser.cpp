/*
 * Author: Broglie 
 * E-mail: yibo141@outlook.com
 */

#include "Parser.h"

Parser::Parser(const std::string request)
{
    assert(request.size() > 0);
    this->_request = request;
}

HTTPRequest Parser::getParseResult()
{
    parseLine();
    parseRequestLine();
    parseHeaders();
    parseBody();
    return _parseResult;
}

void Parser::parseLine()
{
    std::string::size_type lineBegin = 0;   // 正在解析的行的行首索引
    std::string::size_type checkIndex = 0;  // 正在解析的字符索引

    while(checkIndex < _request.size())
    {
        // 如果当前字符是'\r'，则有可能读到一行
        if(_request[checkIndex] == '\r')
        {
            // 如果当前字符是最后一个字符则说明请求没有读取完整
            if((checkIndex + 1) == _request.size())
            {
                std::cout << "Request not to read the complete." << std::endl;
                return;
            }
            // 如果下一个字符是'\n'，则说明我们读取到了完整的一行
            else if(_request[checkIndex+1] == '\n')
            {
                _lines.push_back(std::string(_request, lineBegin,
                            checkIndex - lineBegin));
                checkIndex += 2;
                lineBegin = checkIndex;
            }
            else
            {
                std::cout << "Request error." << std::endl;
                return;
            }
        }
        else
            ++checkIndex;
    }
    return;
}

void Parser::parseRequestLine()
{
    assert(_lines.size() > 0);
    std::string requestLine = _lines[0];

    // first_ws指向请求行的第一个空白字符(' '或'\t')
    auto first_ws = std::find_if(requestLine.cbegin(), requestLine.cend(),
            [](char c)->bool { return (c == ' ' || c == '\t'); });

    // 如果请求行中没有出现空白则请求必定有错误
    if(first_ws == requestLine.cend())
    {
        std::cout << "Request error." << std::endl;
        return;
    }
    // 截取请求方法
    _parseResult.method = std::string(requestLine.cbegin(), first_ws);

    // reverse_last_ws指向请求行中的最后一个空白字符(' '或'\t')
    auto reverse_last_ws = std::find_if(requestLine.crbegin(), requestLine.crend(),
            [](char c)->bool { return (c == ' ' || c == '\t'); });
    auto last_ws = reverse_last_ws.base();
    _parseResult.version = std::string(last_ws, requestLine.cend());

    while((*first_ws == ' ' || *first_ws == '\t') && first_ws != requestLine.cend())
        ++first_ws;

    --last_ws;
    while((*last_ws == ' ' || *last_ws == '\t') && last_ws != requestLine.cbegin())
        --last_ws;

    _parseResult.uri = std::string(first_ws, last_ws + 1);
}

void Parser::parseHeaders()
{
    assert(_lines.size() > 0);
    for(int i = 1; i < _lines.size(); ++i)
    {
        if(_lines[i].empty()) // 如果遇到空行说明请求解析完毕
            return;
        else if(strncasecmp(_lines[i].c_str(), "Host:", 5) == 0) // 处理"Host"头部字段
        {
            auto iter = _lines[i].cbegin() + 5;
            while(*iter == ' ' || *iter == '\t')
                ++iter;
            _parseResult.host = std::string(iter, _lines[i].cend());
        }
        else if(strncasecmp(_lines[i].c_str(), "Connection:", 11) == 0) // 处理"Connection"头部字段
        {
            auto iter = _lines[i].cbegin() + 11;
            while(*iter == ' ' || *iter == '\t')
                ++iter;
            _parseResult.connection = std::string(iter, _lines[i].cend());
        }
        else if(strncasecmp(_lines[i].c_str(), "Authorization:", 14) == 0) // 处理"Connection"头部字段
        {
            auto iter = _lines[i].cbegin() + 14;
            while(*iter == ' ' || *iter == '\t')
                ++iter;
            _parseResult.auth = std::string(iter, _lines[i].cend());
        }
        else
        {
            // 其他头部字段暂时忽略
        }
    }
}


void Parser::parseBody() {
    bool startBody = false;
    _parseResult.body = "";
    for(int i = 0; i < _request.size(); i++){
        if(i + 3 < _request.size()) {
            if(_request[i] == '\r' && _request[i + 1] == '\n' && _request[i + 2] == '\r' && _request[i + 3] == '\n'  ){
                startBody = true;
            }
        }
        if(startBody) {
            _parseResult.body += _request[i];
        }
    }
    
}
