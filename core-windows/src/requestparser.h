#ifndef REQUESTPARSER_H
#define REQUESTPARSER_H

/**
 * @file    requestparser.h
 * @brief   File contains definition of RequestParser
 */

#include <string>
#include <map>

/**
 * @brief Extracts data from HTTP headers, given the data received from client socket
 * @todo Implement extraction of request body (e.g. in POST requests)
 */
class RequestParser
{
    bool half_end_of_line;
    bool end_of_line;
    bool first_line;
    bool beginning;

    std::string method;
    std::string path;
    std::string proto_ver;

    std::string tmp_header_name;
    std::string tmp_header_value;

    char previous_char;

    std::map<std::string, std::string> headers;
    bool headers_available;

    std::string body;
    char prev_char;
    bool beginbody;
    int charcount;
    bool parsingDone;

public:

    /**
     * Initialize a new parser instance.
     */
    RequestParser();

    /**
     * Feeds the parser with next portion of data from client.
     * Should be called in loop, after each chunk of data is received.
     *
     * @param buf Pointer to the buffer in which data from client socket is stored
     * @param size Size of the chunk of data
     */
    void processChunk(const char *buf, size_t size);

    /**
     * Check if the end of headers was reached by the parser.
     *
     * @return Information if all data was already extracted from headers and can be safely accessed
     */
    bool allHeadersAvailable();

    /**
     * Get extracted headers.
     *
     * @return Headers in form of std::map (name -> value)
     */
    std::map<std::string, std::string> getHeaders();

    /**
     * @return String representing the HTTP method used in request
     */
    std::string getMethod();

    /**
     * @return String representing the requested path
     */
    std::string getPath();

    /**
     * @return String representing the protocol (and version) used to perform the request
     */
    std::string getProtocol();

    /**
     * Prepare the object to handle another request (clear all extracted data).
     */
    void reset();

    std::string getBody();

    bool isParsingDone();

    virtual ~RequestParser() = default;
};

#endif // REQUESTPARSER_H
