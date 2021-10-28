#include <iostream>
#include <cstdlib>
#include <string>
#include <regex>
#include <map>
#include <thread>
#include <set>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "lib/json/json.hpp"
#include "settings.h"
#include "server/neuserver.h"
#include "server/router.h"
#include "api/debug/debug.h"
#include "api/events/events.h"

using namespace std;
using json = nlohmann::json;

typedef websocketpp::server<websocketpp::config::asio> websocketserver;
typedef set<websocketpp::connection_hdl, owner_less<websocketpp::connection_hdl>> wsclients;

websocketserver *server;
wsclients connections;

namespace neuserver {

string init() {
    int port = 0;
    json jPort = settings::getOptionForCurrentMode("port");
    if(!jPort.is_null()) {
        port = jPort.get<int>();
    }
    server = new websocketserver();

    server->set_message_handler([&](websocketpp::connection_hdl handler, websocketserver::message_ptr msg) {
        neuserver::handleMessage(handler, msg);
    });
    server->set_http_handler([&](websocketpp::connection_hdl handler) {
        neuserver::handleHTTP(handler);
    });
    server->set_open_handler([&](websocketpp::connection_hdl handler) { 
        connections.insert(handler);
        events::dispatch("clientConnect", connections.size());
    });
    server->set_close_handler([&](websocketpp::connection_hdl handler) {
        connections.erase(handler);
        events::dispatch("clientDisconnect", connections.size());
    });
    
    server->set_access_channels(websocketpp::log::alevel::none);
    server->set_error_channels(websocketpp::log::elevel::none);

    server->init_asio();
    server->set_reuse_addr(true);
    
    string hostAddress = "127.0.0.1";
    if(settings::getMode() == "cloud") {
        hostAddress = "0.0.0.0";
    }
    websocketpp::lib::asio::ip::tcp::endpoint 
        endpoint(websocketpp::lib::asio::ip::address::from_string(hostAddress), port);

    server->listen(endpoint);
    server->start_accept();
    
    websocketpp::lib::asio::error_code error;
    int actualPort = server->get_local_endpoint(error).port();
    
    if(port != actualPort) {
        port = actualPort;
        settings::setPort(port);
    }

    string navigationUrl = "http://localhost:" + std::to_string(port);
    json jUrl = settings::getOptionForCurrentMode("url");

    if(!jUrl.is_null()) {
        string url = jUrl.get<string>();
        if (regex_match(url, regex("^/.*")))
            navigationUrl += url;
        else
            navigationUrl = url;
    }
    return navigationUrl;
}

void startAsync() {
    thread serverThread([&](){ server->run(); });
    serverThread.detach();
}

void stop() {
    server->stop_listening();
}

void handleMessage(websocketpp::connection_hdl handler, websocketserver::message_ptr msg) {
    json nativeMessage;
    try {
        nativeMessage = json::parse(msg->get_payload());
        router::NativeMessage nativeResponse = router::executeNativeMethod({
            nativeMessage["id"].get<string>(),
            nativeMessage["method"].get<string>(),
            nativeMessage["accessToken"].get<string>(),
            nativeMessage["data"]
        });
        
        try {
            json nativeMessage;
            nativeMessage["id"] = nativeResponse.id;
            nativeMessage["method"] = nativeResponse.method;
            nativeMessage["data"] = nativeResponse.data;
            
            server->send(handler, nativeMessage.dump(), msg->get_opcode());
        } catch (websocketpp::exception const & e) {
            debug::log("ERROR", 
                "Unable to send native message: " + std::string(e.what()));
        }
    }
    catch(exception e){
        debug::log("ERROR", 
                "Unable to parse native call payload: " + std::string(e.what()));
    }
}

void handleHTTP(websocketpp::connection_hdl handler) {
    websocketserver::connection_ptr con = server->get_con_from_hdl(handler);
    router::Response routerResponse = router::serve(con->get_resource());
    con->set_status(routerResponse.status);
    con->set_body(routerResponse.data);
    con->replace_header("Content-Type", routerResponse.contentType);
}

void broadcast(const json &message) {
    for (auto connection: connections) {
        server->send(connection, message.dump(), websocketpp::frame::opcode::text);
    }
} 

} // namespace neuserver
