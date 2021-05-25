#include <cstdlib>
#include <string>
#include <thread>
#include "lib/json.hpp"
#include "settings.h"
#include "resources.h"
#include "auth/authbasic.h"
#include "ping/ping.h"
#include "permission.h"
#include "api/app/app.h"
#include "api/window/window.h"
#include "server/serverlistener.h"
#include "lib/easylogging/easylogging++.h"

#define APP_LOG_FILE "/neutralinojs.log"
#define APP_LOG_FORMAT "%level %datetime %msg %loc %user@%host"
INITIALIZE_EASYLOGGINGPP

using namespace std;
using json = nlohmann::json;

int main(int argc, char ** argv) {
    json args;
    bool enableHTTPServer = false;
    for (int i = 0; i < argc; i++) {
        args.push_back(argv[i]);
    }

    settings::setGlobalArgs(args);
    if (!loadResFromDir)
        resources::makeFileTree();
    json options = settings::getConfig();
    authbasic::generateToken();
    ping::startPingReceiver();
    permission::registerBlockList();

    ServerListener *serverListener = new ServerListener();
    string navigationUrl = options["url"];
    if(!options["enableHTTPServer"].is_null())
        enableHTTPServer = options["enableHTTPServer"];
    if(enableHTTPServer) {
        navigationUrl = serverListener->init();
        std::thread serverThread(&ServerListener::run, serverListener);
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
    delete serverListener;
    return 0;
}
