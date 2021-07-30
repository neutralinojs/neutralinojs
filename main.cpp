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
#include "server/ping.h"
#include "auth/permission.h"
#include "server/neuserver.h"
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
        os::MessageBoxOptions msgBoxOptions;
        msgBoxOptions.type = "ERROR";
        msgBoxOptions.title = "Unable to load app configuration";
        msgBoxOptions.content = "neutralino.config.json file is missing or corrupted.";
        os::showMessageBox(msgBoxOptions);
        app::exit();
    }

    authbasic::generateToken();
    ping::startPingReceiver();
    permission::init();

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
        app::open(navigationUrl);
        while(true);
    }
    else if(mode == "window") {
        json windowOptions = options["modes"]["window"];
        windowOptions["url"] = navigationUrl;
        window::controllers::init(windowOptions);
    }
    else if(mode == "cloud") {
        if(enableHTTPServer)
            debug::log("INFO", options["applicationId"].get<std::string>() +
                     " is available at " + navigationUrl);
        while(true);
    }
    delete server;
    return 0;
}
