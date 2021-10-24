#ifndef NEU_SERVER_H
#define NEU_SERVER_H

#include <string>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace neuserver {
    string init();
    void startAsync();
    void stop();
    void handleMessage(websocketpp::connection_hdl handler, websocketpp::server<websocketpp::config::asio>::message_ptr msg);
    void handleHTTP(websocketpp::connection_hdl handler);
    void broadcast(const json &message);
} // namespace neuserver

#endif // #define NEU_SERVER_H
