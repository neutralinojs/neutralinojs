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

#ifndef SERVERLISTENER_H
#define SERVERLISTENER_H

/**
 * @file    serverlistener.h
 * @brief   File contains definition of ServerListener
 */

#include <stdexcept>
#include <string>
#include <functional>

#include <winsock2.h>
#include <ws2tcpip.h>
#include "serverexceptions.h"

/**
 * @brief Creates an HTTP server instance and awaits requests
 *
 * Class uses WinSock library.
 *
 * @warning Server handles only HTTP GET requests (and shows some debug data in response)
 * @warning Threads responsible for handling clients aren't really being taken care of
 * @todo Handle more HTTP methods
 * @todo Give the opportunity to handle client requests to the user (some kind of routing)
 */
class ServerListener {
    int port;
    size_t buffer_size;
    SOCKET listen_socket = INVALID_SOCKET;
    bool server_running;

    static void clientHandler(SOCKET client_socket, size_t buffer_size);

public:

    /**
     * Initialize a new server instance.
     *
     * @param port Port on which the server will be listening for connections
     * @param buffer_size Size of the buffer used to retrieve data from sockets
     */
    ServerListener(int port=8080, size_t buffer_size=255);

    /**
      * Start listening for connections.
      *
      * @param client_acceptation_error_callback The function receiving the ClientAcceptationException object when a problem with acceptation of new connection occurs
      */
    void run(std::function<void(ClientAcceptationException)> client_acceptation_error_callback = [](ClientAcceptationException) {});

    /**
     * Stop listening for connections (close listening socket).
     *
     * @warning It could be run only from the different thread, as start() is blocking
     */
    void stop();

    virtual ~ServerListener();
};

#endif // SERVERLISTENER_H
