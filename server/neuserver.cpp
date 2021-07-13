#include <iostream>
#include <cstdlib>
#include <string>
#include <regex>
#include <map>
#include "lib/json.hpp"
#include "lib/httplib.h"
#include "server/neuserver.h"
#include "settings.h"
#include "server/router.h"

using namespace std;
using json = nlohmann::json;

std::string NeuServer::init() {
    this->svr = new httplib::Server();

    this->svr->Get(".*", [](const httplib::Request &req, httplib::Response &res) {
      router::Response routerResponse  =  router::handle({req.path, "", req.get_header_value("Authorization")});
      res.set_content(routerResponse.data, routerResponse.header.c_str());
    });

    this->svr->Post(".*", [](const httplib::Request &req, httplib::Response &res) {
      router::Response routerResponse =  router::handle({req.path, req.body, req.get_header_value("Authorization")});
      res.set_content(routerResponse.data, routerResponse.header.c_str());
    });

    json options = settings::getConfig();
    string mode = settings::getMode();

    if(!options["port"].is_null())
        this->port = options["port"];
        
    if(mode == "cloud")
        this->hostName = "0.0.0.0";

    if(this->port == 0) {
        this->port = this->svr->bind_to_any_port(this->hostName.c_str());
        settings::setPort(this->port);
        this->usingRandomPort = true;
    }

    string navigationUrl = "http://localhost:" + std::to_string(port);
    if(!options["url"].is_null()) {
        string url = options["url"];
        if (regex_match(url, regex("^/.*")))
            navigationUrl += url;
        else
            navigationUrl = url;
    }

    return navigationUrl;
}

void NeuServer::run() {
    if(this->usingRandomPort)
        this->svr->listen_after_bind();
    else
        this->svr->listen(this->hostName.c_str(), this->port);
}

void NeuServer::stop() {
    delete this->svr;
}

