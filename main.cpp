#include <cstdlib>
#include <string>
#include <thread>
#if defined(_WIN32)
#include <winsock2.h>
#endif

#include "lib/json.hpp"
#include "lib/easylogging/easylogging++.h"
#include "lib/portable-file-dialogs.h"
#include "auth/permission.h"
#include "auth/authbasic.h"
#include "server/neuserver.h"
#include "server/ping.h"
#include "settings.h"
#include "resources.h"
#include "api/app/app.h"
#include "api/window/window.h"
#include "api/os/os.h"
#include "api/debug/debug.h"

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
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
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
    if (!loadResFromDir) {
        bool resourceLoaderStatus = resources::makeFileTree();
        if(!resourceLoaderStatus)
            loadResFromDir = true;
    }

    json options = settings::getConfig();
    if(options.is_null()) {
        pfd::message("Unable to load app configuration",
                        "neutralino.config.json file is missing or corrupted.",
                        pfd::choice::ok,
                        pfd::icon::error);
        app::exit();
    }

    authbasic::generateToken();
    ping::startPingReceiver();
    permission::init();

    NeuServer *server = NeuServer::getInstance();
    string navigationUrl = settings::getOptionForCurrentMode("url").get<string>();
    json jEnableHTTPServer = settings::getOptionForCurrentMode("enableHTTPServer");

    if(!jEnableHTTPServer.is_null())
        enableHTTPServer = jEnableHTTPServer.get<bool>();
    if(enableHTTPServer) {
        navigationUrl = server->init();
        thread serverThread([&](){ server->run(); });
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
        os::open(navigationUrl);
        while(true);
    }
    else if(mode == "window") {
        json windowOptions = options["modes"]["window"];
        windowOptions["url"] = navigationUrl;
        window::controllers::init(windowOptions);
    }
    else if(mode == "cloud") {
        if(enableHTTPServer)
            debug::log("INFO", options["applicationId"].get<string>() +
                     " is available at " + navigationUrl);
        while(true);
    }
    delete server;
    return 0;
}
