#include <regex>
#include <winsock2.h>
#include "settings.h"
#include "resources.h"
#include "auth/authbasic.h"
#include "ping/ping.h"
#include "permission.h"
#include "lib/json.hpp"
#include "server/serverlistener.h"
#include "api/app/app.h"

using namespace std;
using json = nlohmann::json;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow) {

    json args;
    for(int i = 0; i < __argc; i++) {
        args.push_back(__argv[i]);
    }
    settings::setGlobalArgs(args);
    if(!loadResFromDir)
        resources::makeFileTree();
    json options = settings::getConfig();
    authbasic::generateToken();
    ping::startPingReceiver();
    permission::registerBlockList();

    int port = 0;
    if(!options["port"].is_null())
        port = options["port"];
    string mode = settings::getMode();
    ServerListener serverListener(port);

    serverListener.init();

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if(mode == "cloud")
        sin.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (getsockname(serverListener.listen_socket, (struct sockaddr *)&sin, &len) == -1) {
        perror("getsockname");
    }
    else {
        port = ntohs(sin.sin_port);
        settings::setPort(port);
    }
    string navigationUrl = "http://localhost:" + std::to_string(port);
    if(!options["url"].is_null()) {
        string url = options["url"];
        if (regex_match(url, regex("^/.*")))
            navigationUrl += url;
        else
            navigationUrl = url;
    }

    if(mode == "browser") {
        json browserOptions = options["modes"]["browser"];
        browserOptions["url"] = navigationUrl;
        app::open(browserOptions);
    }
    else if(mode == "window") {
        json windowOptions = options["modes"]["window"];
        windowOptions["url"] = navigationUrl;
        app::showWindow(windowOptions);
    }
    serverListener.run();
    return 0;
}

