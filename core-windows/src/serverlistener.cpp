#include "serverlistener.h"

#include <iostream>
#include <cstdlib>
#include <list>
#include <future>
#include <chrono>
#include <thread>
#include <sstream>
#include <fstream>
#include "settings.h"
#include "requestparser.h"
#include "router.h"




ServerListener::ServerListener(int port, size_t buffer_size) {
    this->port = port;
    this->buffer_size = buffer_size;
    this->server_running = false;
    this->listen_socket = INVALID_SOCKET;

    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw ServerStartupException();
    }
}

void ServerListener::run(std::function<void(ClientAcceptationException)> client_acceptation_error_callback) {

    settings::getSettings();
    json options = settings::getOptions();
    string appname = options["appname"];
    string appport = options["appport"];
    this->port = stoi(appport);

    std::shared_ptr<addrinfo> socket_props(nullptr, [](addrinfo* ai) { freeaddrinfo(ai); });
    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int addrinfo_status = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, (addrinfo**)&socket_props);
    if(addrinfo_status != 0) {
        throw AddrinfoException(addrinfo_status);
    }

    listen_socket = socket(socket_props->ai_family, socket_props->ai_socktype, socket_props->ai_protocol);
    if(listen_socket == INVALID_SOCKET) {
        throw SocketCreationException(WSAGetLastError());
    }

    if(bind(listen_socket, socket_props->ai_addr, (int)socket_props->ai_addrlen) == SOCKET_ERROR) {
        closesocket(listen_socket);
        std::cout << "Neutralino is already running on " << DEFAULT_PORT << std::endl; 
        ShellExecute(0, 0, ("http://localhost:" + appport + "/" + appname).c_str(), 0, 0 , SW_SHOW );
        std::exit(0);
        //throw SocketBindingException(WSAGetLastError());
    }

    if(listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(listen_socket);
        throw ListenException(WSAGetLastError());
    }

    std::map<SOCKET, std::thread> threads;

    bool server_running = true;
    ShellExecute(0, 0, ("http://localhost:" + appport + "/" + appname).c_str(), 0, 0 , SW_SHOW );
    while(server_running) {
        SOCKET client_socket;
        try {
            client_socket = accept(listen_socket, NULL, NULL);
            if(client_socket == INVALID_SOCKET) {
                throw ClientAcceptationException(WSAGetLastError());
            }
        } catch(ClientAcceptationException &e) {
            client_acceptation_error_callback(e);
            continue;
        }
        threads[client_socket] = std::thread(ServerListener::clientHandler, client_socket, buffer_size);
        threads[client_socket].detach();
    }
}

void ServerListener::stop() {
    server_running = false;
    if(listen_socket != INVALID_SOCKET) {
        shutdown(listen_socket, SD_BOTH);
        closesocket(listen_socket);
    }
}

void ServerListener::clientHandler(SOCKET client_socket, size_t buffer_size) {
    int recvbuflen = buffer_size;
    char recvbuf[recvbuflen];
    int bytes_received;
    RequestParser parser;

    sockaddr_in client_info;
    int client_info_len = sizeof(sockaddr_in);
    char *client_ip;

    if(getpeername(client_socket, (sockaddr*)(&client_info), &client_info_len) == SOCKET_ERROR) {
        goto cleanup;
    }
    client_ip = inet_ntoa(client_info.sin_addr);

    while(1) {
        parser.reset();

        bool headers_ready = false;
        while(!parser.isParsingDone()) {
            bytes_received = recv(client_socket, recvbuf, recvbuflen, 0);
            if(bytes_received > 0) {
                parser.processChunk(recvbuf, bytes_received);
                if(parser.allHeadersAvailable()) {
                    headers_ready = true;
                }
            } else {
                goto cleanup;
            }
        }
        
        std::string response_body = "";
        pair<string, string> responseGen =  routes::handle(parser.getPath(), parser.getBody());
        response_body = responseGen.first;

        std::string response_headers = "HTTP/1.1 200 OK\r\n"
        "Content-Type: " + responseGen.second + "; charset=UTF-8\r\n"
        "Connection: keep-alive\r\n"
        "Content-Length: " + std::to_string(response_body.length()) + "\r\n\r\n";

        std::string response = response_headers + response_body;
        send(client_socket, response.c_str(), strlen(response.c_str()), 0);
    }
cleanup:
    closesocket(client_socket);
}

ServerListener::~ServerListener() {
    WSACleanup();
}
