#include <cstdlib>
#include <string>
#include <thread>
#if defined(_WIN32)
#include <winsock2.h>
#include <websocketpp/error.hpp>
#include <WinBase.h>
#define PIPE_NAME TEXT("\\\\.\\pipe\\neutralino")
#endif

#include "lib/json/json.hpp"
#include "lib/easylogging/easylogging++.h"
#include "lib/filedialogs/portable-file-dialogs.h"
#include "auth/permission.h"
#include "auth/authbasic.h"
#include "server/neuserver.h"
#include "settings.h"
#include "resources.h"
#include "helpers.h"
#include "chrome.h"
#include "extensions_loader.h"
#include "api/app/app.h"
#include "api/window/window.h"
#include "api/os/os.h"
#include "api/debug/debug.h"
#include "api/events/events.h"

#define NEU_APP_LOG_FILE "/neutralinojs.log"
#define NEU_APP_LOG_FORMAT "%level %datetime %msg %loc %user@%host"
#define ELPP_THREAD_SAFE

INITIALIZE_EASYLOGGINGPP

using namespace std;
using json = nlohmann::json;

string navigationUrl = "";

bool __checkSingleInstance() {
    #if defined(_WIN32)
    CreateMutexA(NULL, TRUE, "neutralino_mutex_lock");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return false;
    }
    return true;
    #endif
    //TODO check other os
    return true;
}

void __checkForSingleInstanceSignal() {
    #if defined(_WIN32)
    HANDLE hPipe;
    char buffer[2048];
    DWORD dwRead;

    hPipe = CreateNamedPipe(PIPE_NAME,
                            PIPE_ACCESS_DUPLEX,
                            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 
                            1,
                            1024 * 16,
                            1024 * 16,
                            NMPWAIT_USE_DEFAULT_WAIT,
                            NULL);
    while (hPipe != INVALID_HANDLE_VALUE)
    {
        if (ConnectNamedPipe(hPipe, NULL) != FALSE)
        {
            while (ReadFile(hPipe, buffer, sizeof(buffer), &dwRead, NULL) != FALSE)
            {
                std::string bufferString(buffer, dwRead);
                json bufferJson = bufferString;

                events::dispatch("otherInstance", bufferJson);
            }
        }

        DisconnectNamedPipe(hPipe);
    }
    #endif
    //TODO other os
}

void __sendArgsToFirstInstance(json args) {
    #if defined(_WIN32)
    HANDLE hPipe;
    DWORD dwWritten;

    hPipe = CreateFile(PIPE_NAME, 
                       GENERIC_READ | GENERIC_WRITE, 
                       0,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);
    if (hPipe != INVALID_HANDLE_VALUE)
    {
        std::string buffer = args.dump();
        WriteFile(hPipe,
                  buffer.c_str(),
                  buffer.size(),
                  &dwWritten,
                  NULL);

        CloseHandle(hPipe);
    }
    #endif
    //TODO
}

void __wait() {
    while(true) {
        this_thread::sleep_for(20000ms);
    }
}

void __startApp(json args) {
    json options = settings::getConfig();
    switch(settings::getMode()) {
        case settings::AppModeBrowser:
            os::open(navigationUrl);
            __wait();
            break;
        case settings::AppModeWindow: {
            json windowOptions = options["modes"]["window"];
            if(helpers::hasField(windowOptions, "singleInstance")) {
                bool singleInstance = windowOptions["singleInstance"].get<bool>();

                if(singleInstance) {
                    if(__checkSingleInstance()) {
                        thread t = thread(__checkForSingleInstanceSignal);
                        t.detach();
                    }
                    else {
                        __sendArgsToFirstInstance(args);
                        return;
                    }
                }
            }
            windowOptions["url"] = navigationUrl;
            window::controllers::init(windowOptions);
            }
            break;
        case settings::AppModeCloud:
            if(neuserver::isInitialized()) {
                debug::log(debug::LogTypeInfo, settings::getAppId() +
                        " is available at " + navigationUrl);
            }
            __wait();
            break;
        case settings::AppModeChrome: {
            json chromeOptions = options["modes"]["chrome"];
            chromeOptions["url"] = navigationUrl;
            chrome::init(chromeOptions);
            __wait();
            }
            break;
    }
}

void __configureLogger() {
    bool enableLogging = true;
    bool enableLogFile = true;

    json logging = settings::getOptionForCurrentMode("logging");
    if(!logging["enabled"].is_null()) {
        enableLogging = logging["enabled"].get<bool>();
    }
    if(!logging["writeToLogFile"].is_null()) {
        enableLogFile = logging["writeToLogFile"].get<bool>();
    }

    el::Configurations defaultConf;
    defaultConf.setToDefault();
    defaultConf.setGlobally(
            el::ConfigurationType::Format, NEU_APP_LOG_FORMAT);

    if(enableLogFile) {
        defaultConf.setGlobally(
                el::ConfigurationType::Filename, settings::joinAppPath(NEU_APP_LOG_FILE));
    }
    defaultConf.setGlobally(
            el::ConfigurationType::ToFile, enableLogFile ? "true" : "false");

    defaultConf.setGlobally(
            el::ConfigurationType::Enabled, enableLogging ? "true" : "false");
    el::Loggers::reconfigureLogger("default", defaultConf);
}

void __startServerAsync() {
    navigationUrl = settings::getNavigationUrl();
    json jEnableServer = settings::getOptionForCurrentMode("enableServer");

    if(!jEnableServer.is_null() && jEnableServer.get<bool>()) {
        try {
            navigationUrl = neuserver::init();
        }
        catch(websocketpp::exception &e) {
            json jPort = settings::getOptionForCurrentMode("port");
            string errorMsg = "Neutralinojs can't initialize the application server";
            if(!jPort.is_null()) {
                errorMsg += " on port: " + to_string(jPort.get<int>());
            }
            pfd::message("Unable to start server",
                errorMsg,
                pfd::choice::ok,
                pfd::icon::error);
            std::exit(1);
        }
        neuserver::startAsync();
    }
}

void __initFramework(const json &args) {
    settings::setGlobalArgs(args);
    resources::init();
    bool settingsStatus = settings::init();
    if(!settingsStatus) {
        pfd::message("Unable to load configuration",
            "The application configuration file cannot be loaded due to a JSON parsing error.",
            pfd::choice::ok,
            pfd::icon::error);
        std::exit(1);
    }
    authbasic::init();
    permission::init();
}

void __initExtra() {
    bool enableExtensions = false;
    bool exportAuthInfo = false;
    json exts = settings::getOptionForCurrentMode("enableExtensions");
    if(!exts.is_null()) {
        enableExtensions = exts.get<bool>();
    }
    json exportAuth = settings::getOptionForCurrentMode("exportAuthInfo");
    if(!exportAuth.is_null()) {
        exportAuthInfo = exportAuth.get<bool>();
    }

    if(exportAuthInfo) {
        authbasic::exportAuthInfo();
    }
    if(enableExtensions) {
        extensions::init();
    }
}

#if defined(_WIN32)
#define ARG_C __argc
#define ARG_V __wargv
#define CONVSTR(S) helpers::wcstr2str(S)
int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPTSTR    lpCmdLine,
                      int       nCmdShow)
                      {
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#define ARG_C argc
#define ARG_V argv
#define CONVSTR(S) S
int main(int argc, char ** argv)
{
#endif
    json args;
    for (int i = 0; i < ARG_C; i++) {
        args.push_back(CONVSTR(ARG_V[i]));
    }
    __initFramework(args);
    __startServerAsync();
    __configureLogger();
    __initExtra();
    __startApp(args);
    return 0;
}
