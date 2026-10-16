#include <string>
#include <regex>
#include <map>
#include <thread>
#include <chrono>
#include <set>
#include <fstream>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "lib/json/json.hpp"
#include "settings.h"
#include "helpers.h"
#include "errors.h"
#include "extensions_loader.h"
#include "server/neuserver.h"
#include "server/router.h"
#include "auth/authbasic.h"
#include "api/debug/debug.h"
#include "api/events/events.h"
#include "api/app/app.h"
#include "api/fs/fs.h"


using namespace std;
using json = nlohmann::json;

typedef websocketpp::server<websocketpp::config::asio> websocketserver;
typedef map<string, websocketpp::connection_hdl> wsclientsMap;
typedef set<websocketpp::connection_hdl, owner_less<websocketpp::connection_hdl>> wsclientsSet;

namespace neuserver {

websocketserver *server;
wsclientsSet appConnections;
wsclientsMap extConnections;
map<websocketpp::connection_hdl,
    websocketpp::frame::opcode::value,
    owner_less<websocketpp::connection_hdl>> wsMode;

string serverHost = "127.0.0.1";
bool initialized = false;
bool applyConfigHeaders = false;

bool __isExtensionEndpoint(const string &url) {
    return regex_match(url, regex(".*extensionId=.*"));
}

bool __hasConnectToken(const string &url) {
    return regex_match(url, regex(".*connectToken=.*"));
}

fs::FileReaderOptions __getFileReaderOptionsFromHeaders(const websocketpp::http::parser::header_list &headers) {
    fs::FileReaderOptions fileReaderOptions;
    auto rangeIt = headers.find("Range");
    if(rangeIt == headers.end()) {
        return fileReaderOptions;
    }

    smatch match;
    if(regex_match(rangeIt->second, match, regex(R"(bytes=(\d+)-(\d*)?)"))) {
        fileReaderOptions.pos = stoll(match[1].str());
        if(match[2].matched && match[2].str() != "") {
            long long end = stoll(match[2].str());
            if(end >= fileReaderOptions.pos) {
                fileReaderOptions.size = end - fileReaderOptions.pos + 1;
            }
        }
    }
    return fileReaderOptions;
}

void __applyConfigHeaders(websocketserver::connection_ptr con) {
    json jHeaders = settings::getOptionForCurrentMode("serverHeaders");
    for(const auto &it: jHeaders.items()) {
        con->replace_header(it.key(), it.value().get<string>());
    }
}

string __getParamValueFromUrl(const string &url, const string &param) {
    string val = "";
    smatch matches;
    if(regex_search(url, matches, regex(param + "=([\\w.\\-_]+)"))) {
        if(matches.size() >= 2) {
            val = matches[1].str();
        }
    }
    return val;
}

string __getExtensionIdFromUrl(const string &url) {
    return __getParamValueFromUrl(url, "extensionId");
}

string __getConnectTokenFromUrl(const string &url) {
    return __getParamValueFromUrl(url, "connectToken");
}

void __exitProcessIfIdle() {
    thread exitCheckThread([=]() {
        this_thread::sleep_for(10s);
        if(appConnections.size() == 0) {
            app::exit();
        }
    });
    exitCheckThread.detach();
}

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
        neuserver::handleConnect(handler);
    });

    server->set_close_handler([&](websocketpp::connection_hdl handler) {
        neuserver::handleDisconnect(handler);
    });

    server->set_validate_handler([&](websocketpp::connection_hdl handler) {
        return neuserver::handleValidate(handler);
    });

    server->set_access_channels(websocketpp::log::alevel::none);
    server->set_error_channels(websocketpp::log::elevel::none);

    server->init_asio();
    server->set_reuse_addr(true);

    // Security fix for GHSA-fp96-mrqq-p5rq (CWE-1327): Binding to an Unrestricted IP Address
    //
    // Threat Model:
    // In cloud mode, Neutralino acts as a headless server, defaulting to listening on all interfaces
    // (0.0.0.0) so external devices (mobile phones, IoT, Docker hosts) can connect. However,
    // because static HTTP file serving does not require a WebSocket token, listening on 0.0.0.0
    // exposes the app's local assets and endpoints to any device on the network without any warning.
    //
    // Defense:
    // 1. Maintain 127.0.0.1 (loopback) default for desktop modes (window, browser, chrome).
    // 2. Allow developers to explicitly configure the bind interface via "host" in config or "--host" CLI option
    //    (e.g. allowing developers to restrict cloud mode to 127.0.0.1 for local container/daemon isolation).
    // 3. When binding to an external/unrestricted interface (0.0.0.0 or non-loopback), emit a bold security
    //    warning recommending caution and informing developers how to restrict it.
    string hostAddress = (settings::getMode() == settings::AppModeCloud) ? "0.0.0.0" : "127.0.0.1";
    json jHost = settings::getOptionForCurrentMode("host");
    if(!jHost.is_null() && !jHost.get<string>().empty()) {
        hostAddress = jHost.get<string>();
    }

    websocketpp::lib::asio::error_code addrError;
    auto ipAddress = websocketpp::lib::asio::ip::address::from_string(hostAddress, addrError);
    if(addrError) {
        debug::log(debug::LogTypeError, "Invalid host address '" + hostAddress + "'; falling back to 127.0.0.1");
        hostAddress = "127.0.0.1";
        ipAddress = websocketpp::lib::asio::ip::address::from_string("127.0.0.1");
    }

    if(hostAddress != "127.0.0.1" && hostAddress != "localhost" && hostAddress != "::1") {
        debug::log(debug::LogTypeWarning, "SECURITY WARNING: Server is listening on an external network interface (" + hostAddress +
            "). Static files and server endpoints are accessible to other devices on the network. Configure firewall rules or use '--host=127.0.0.1' for local isolation.");
    }
    serverHost = hostAddress;

    websocketpp::lib::asio::ip::tcp::endpoint endpoint(ipAddress, port);

    server->listen(endpoint);
    server->start_accept();

    websocketpp::lib::asio::error_code error;
    int actualPort = server->get_local_endpoint(error).port();

    if(port != actualPort) {
        port = actualPort;
        settings::setPort(port);
    }

    string navigationUrl = "http://127.0.0.1:" + to_string(port);
    json jUrl = settings::getOptionForCurrentMode("url");

    if(!jUrl.is_null()) {
        string url = jUrl.get<string>();
        if (regex_match(url, regex("^/.*")))
            navigationUrl += url;
        else
            navigationUrl = url;
    }
    initialized = true;

    json jHeaders = settings::getOptionForCurrentMode("serverHeaders");
    if(!jHeaders.is_null()) {
        applyConfigHeaders = true;
    }

    return navigationUrl;
}

bool isInitialized() {
    return initialized;
}

void startAsync() {
    thread serverThread([&](){ server->run(); });
    serverThread.detach();
}

void stop() {
    server->stop_listening();
}

void handleMessage(websocketpp::connection_hdl handler, websocketserver::message_ptr msg) {
    websocketserver::connection_ptr con = server->get_con_from_hdl(handler);
    string url = con->get_resource();
    if(__isExtensionEndpoint(url) && wsMode.find(handler) == wsMode.end()) {
        if(msg->get_opcode() == websocketpp::frame::opcode::text) {
        	wsMode[handler] = websocketpp::frame::opcode::text;
        }
        else if(msg->get_opcode() == websocketpp::frame::opcode::binary) {
        	wsMode[handler] = websocketpp::frame::opcode::binary;
        }
    }
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
            json responseMessage;
            responseMessage["id"] = nativeResponse.id;
            responseMessage["method"] = nativeResponse.method;
            responseMessage["data"] = nativeResponse.data;

            auto op = websocketpp::frame::opcode::text;

            if(__isExtensionEndpoint(url) && wsMode.count(handler)) {
                op = wsMode[handler];
            }

            server->send(handler, helpers::jsonToString(responseMessage), op);
        } catch (websocketpp::exception const & e) {
            debug::log(debug::LogTypeError, errors::makeErrorMsg(errors::NE_SR_UNBSEND));
        }
    }
    catch(const exception& e) {
        debug::log(debug::LogTypeError, errors::makeErrorMsg(errors::NE_SR_UNBPARS));
    }
}

void handleHTTP(websocketpp::connection_hdl handler) {
    websocketserver::connection_ptr con = server->get_con_from_hdl(handler);
    string resource = con->get_resource();
    string documentRoot = neuserver::getDocumentRoot();
    if(!documentRoot.empty()) {
        resource = documentRoot + resource;
    }
    fs::FileReaderOptions fileReaderOptions =
        __getFileReaderOptionsFromHeaders(con->get_request().get_headers());
    router::Response routerResponse = router::serve(resource, fileReaderOptions);
    con->set_status(routerResponse.status);
    con->set_body(routerResponse.data);
    con->replace_header("Content-Type",routerResponse.contentType);
    for(const auto &[header, value]: routerResponse.headers) {
        con->replace_header(header, value);
    }
    
    if(applyConfigHeaders){
        __applyConfigHeaders(con);
    }
}

void handleConnect(websocketpp::connection_hdl handler) {
    websocketserver::connection_ptr con = server->get_con_from_hdl(handler);
    string url = con->get_resource();
    if(__isExtensionEndpoint(url)) {
        string extensionId = __getExtensionIdFromUrl(url);
        extConnections[extensionId] = handler;
        events::dispatch("extClientConnect", extensionId);
    }
    else {
        appConnections.insert(handler);
        events::dispatch("appClientConnect", appConnections.size());
    }
    events::dispatch("clientConnect", appConnections.size() + extConnections.size());
}

void handleDisconnect(websocketpp::connection_hdl handler) {
    wsMode.erase(handler);
    
    websocketserver::connection_ptr con = server->get_con_from_hdl(handler);
    string url = con->get_resource();
    if(__isExtensionEndpoint(url)) {
        string extensionId = __getExtensionIdFromUrl(url);
        extConnections.erase(extensionId);
        events::dispatch("extClientDisconnect", extensionId);
    }
    else {
        settings::AppMode mode = settings::getMode();
        appConnections.erase(handler);
        if(mode == settings::AppModeBrowser || mode == settings::AppModeChrome) {
            __exitProcessIfIdle();
        }
        events::dispatch("appClientDisconnect", appConnections.size());
    }
    events::dispatch("clientDisconnect", appConnections.size() + extConnections.size());
}

bool handleValidate(websocketpp::connection_hdl handler) {
    websocketserver::connection_ptr con = server->get_con_from_hdl(handler);
    string url = con->get_resource();
    string host = con->get_host();

    if(!(host == "localhost" || host == "127.0.0.1" || host == serverHost || serverHost == "0.0.0.0") && settings::getMode() != settings::AppModeCloud) {
        return false;
    }

    if(!__hasConnectToken(url)) {
        return false;
    }

    string connectToken = __getConnectTokenFromUrl(url);
    if(!authbasic::verifyConnectToken(connectToken)) {
        return false;
    }

    if(__isExtensionEndpoint(url)) {
        string extensionId = __getExtensionIdFromUrl(url);
        return extensions::isLoaded(extensionId);
    }

    return true;
}

void broadcast(const json &message) {
    neuserver::broadcastToAllApps(message);
    neuserver::broadcastToAllExtensions(message); 
}

bool sendToExtension(const string &extensionId, const json &message) {
    if(extConnections.find(extensionId) != extConnections.end()) {
        auto hdl = extConnections[extensionId];
        auto op = wsMode.count(hdl)
              ? wsMode[hdl]
              : websocketpp::frame::opcode::text;
        server->send(hdl, helpers::jsonToString(message), op);
        return true;
    }
    return false;
}

void broadcastToAllExtensions(const json &message) {
    for (const auto &[_, connection]: extConnections) {
        auto op = wsMode.count(connection)
            ? wsMode[connection]
            : websocketpp::frame::opcode::text;
        server->send(connection, helpers::jsonToString(message), op);
    }
}

void broadcastToAllApps(const json &message) {
    for (const auto &connection: appConnections) {
        server->send(connection, helpers::jsonToString(message), websocketpp::frame::opcode::text);
    }
}

vector<string> getConnectedExtensions() {
    vector<string> extensions;
    for (const auto &[extensionId, _]: extConnections) {
        extensions.push_back(extensionId);
    }
    return extensions;
}

string getDocumentRoot() {
    string documentRoot = "";
    json jDocumentRoot = settings::getOptionForCurrentMode("documentRoot");
    if(!jDocumentRoot.is_null()) {
        documentRoot = jDocumentRoot.get<string>();
        
        if(documentRoot.back() == '/') {
            documentRoot.pop_back();
        }
    }
    return documentRoot;
}

string getHostAddress() {
    return serverHost;
}

} // namespace neuserver
