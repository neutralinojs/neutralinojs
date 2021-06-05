#include <cstdlib>
#if defined(_WIN32)
#include <winsock2.h>
#endif
#include <string>
#include <thread>
#include "lib/json.hpp"
#include "lib/easylogging/easylogging++.h"
#include "settings.h"
#include "resources.h"
#include "auth/authbasic.h"
#include "ping/ping.h"
#include "permission.h"
#include "server/neuserver.h"

#if defined(__linux__)
#include "../core-linux/src/api/app/app.h"
#include "../core-linux/src/api/window/window.h"

#elif defined(_WIN32)
#include "../core-windows/src/api/app/app.h"
#include "../core-windows/src/api/window/window.h"

#elif defined(__APPLE__)
#include "../core-macos/src/api/app/app.h"
#include "../core-macos/src/api/window/window.h"
#endif

#define APP_LOG_FILE "/neutralinojs.log"
#define APP_LOG_FORMAT "%level %datetime %msg %loc %user@%host"
INITIALIZE_EASYLOGGINGPP

using namespace std;
using json = nlohmann::json;

#if defined(_WIN32)
#define ARG_C __argc
#define ARG_V __argv 
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
#else
#define ARG_C argc
#define ARG_V argv 
int main(int argc, char ** argv)
#endif
                                {
    json args;
    for (int i = 0; i < ARG_C; i++) {
        args.push_back(ARG_V[i]);
    }
    bool enableHTTPServer = false;
    settings::setGlobalArgs(args);
    if (!loadResFromDir)
        resources::makeFileTree();
    json options = settings::getConfig();
    authbasic::generateToken();
    ping::startPingReceiver();
    permission::registerBlockList();

    NeuServer *server = new NeuServer();
    string navigationUrl = options["url"];
    if(!options["enableHTTPServer"].is_null())
        enableHTTPServer = options["enableHTTPServer"];
    if(enableHTTPServer) {
        navigationUrl = server->init();
        std::thread serverThread([&](){ server->run(); });
        serverThread.detach();
    }

    el::Configurations defaultConf;
    defaultConf.setToDefault();
    defaultConf.setGlobally(
            el::ConfigurationType::Filename, settings::joinAppPath(APP_LOG_FILE));
    defaultConf.setGlobally(
            el::ConfigurationType::Format, APP_LOG_FORMAT);
    defaultConf.setGlobally(
            el::ConfigurationType::ToFile, "TRUE");
    el::Loggers::reconfigureLogger("default", defaultConf);

    string mode = settings::getMode();
    if(mode == "browser") {
        json browserOptions = options["modes"]["browser"];
        browserOptions["url"] = navigationUrl;
        app::open(browserOptions);
        while(true);
    }
    else if(mode == "window") {
        json windowOptions = options["modes"]["window"];
        windowOptions["url"] = navigationUrl;
        window::show(windowOptions);
    }
    else if(mode == "cloud") {
        if(enableHTTPServer)
            LOG(INFO) << options["applicationId"].get<std::string>() <<
                     " is available at " << navigationUrl;
        while(true);
    }
    delete server;
    return 0;
}
