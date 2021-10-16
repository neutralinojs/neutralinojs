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

bool enableServer = false;
string navigationUrl = "";

void startApp() {
    json options = settings::getConfig();
    string mode = settings::getMode();
    if(mode == "browser") {
        ping::start();
        os::open(navigationUrl);
        while(true);
    }
    else if(mode == "window") {
        json windowOptions = options["modes"]["window"];
        windowOptions["url"] = navigationUrl;
        window::controllers::init(windowOptions);
    }
    else if(mode == "cloud") {
        if(enableServer)
            debug::log("INFO", options["applicationId"].get<string>() +
                     " is available at " + navigationUrl);
        while(true);
    }
}

void configureLogger() {
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    defaultConf.setGlobally(
            el::ConfigurationType::Filename, settings::joinAppPath(APP_LOG_FILE));
    defaultConf.setGlobally(
            el::ConfigurationType::Format, APP_LOG_FORMAT);
    defaultConf.setGlobally(
            el::ConfigurationType::ToFile, "TRUE");
    el::Loggers::reconfigureLogger("default", defaultConf);
}

void startServerAsync() {
    navigationUrl = settings::getOptionForCurrentMode("url").get<string>();
    json jEnableServer = settings::getOptionForCurrentMode("enableServer");

    if(!jEnableServer.is_null() && jEnableServer.get<bool>()) {
        enableServer = true;
        navigationUrl = neuserver::init();
        neuserver::startAsync();
    }
}

void initFramework(json args) {
    settings::setGlobalArgs(args);
    json options = settings::getConfig();
    if(options.is_null()) {
        pfd::message("Unable to load app configuration",
                        "neutralino.config.json file is missing or corrupted.",
                        pfd::choice::ok,
                        pfd::icon::error);
        app::exit();
    }
    if (!loadResFromDir) {
        bool resourceLoaderStatus = resources::makeFileTree();
        if(!resourceLoaderStatus)
            loadResFromDir = true;
    }

    authbasic::generateToken();
    permission::init();
}


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
    initFramework(args);
    startServerAsync();
    configureLogger();
    startApp();
    return 0;
}
