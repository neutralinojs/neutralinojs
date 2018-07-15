/*
 * Author: Broglie 
 * E-mail: yibo141@outlook.com
 */

#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <string.h>

// 解析请求后的数据存储在http_request结构体中
typedef struct
{
    std::string method;     // 请求的方法
    std::string uri;        // 请求的uri
    std::string version;    // HTTP版本
    std::string host;       // 请求的主机名
    std::string connection; // Connection首部
    std::string auth;
    std::string body;
} HTTPRequest;

class Parser
{
public:
    Parser(const std::string request);
    HTTPRequest getParseResult();
private:
    void parseLine();        // 将请求按行解析存入_lines数组中
    void parseRequestLine(); // 解析请求行
    void parseHeaders();     // 解析头部字段
    void parseBody(); // parser exnteded for POST body
    std::string _request;    // 客户的原始请求
    std::vector<std::string> _lines;
    HTTPRequest _parseResult;
};

#endif // PARSER_H