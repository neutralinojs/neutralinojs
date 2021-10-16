#ifndef NEUSERVER_H
#define NEUSERVER_H

#include <string>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "lib/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace neuserver {
    string init();
    void startAsync();
    void stop();
    void handle(websocketpp::connection_hdl handler, websocketpp::server<websocketpp::config::asio>::message_ptr msg);
    void handleHTTP(websocketpp::connection_hdl handler);
    void broadcast(json message);
};

#endif // NEUSERVER_H
