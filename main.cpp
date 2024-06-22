#include <cstdlib>
#include <string>
#include <thread>
#if defined(_WIN32)
#include <winsock2.h>
#include <websocketpp/error.hpp>
#include <WinBase.h>
#define PIPE_NAME TEXT("\\\\.\\pipe\\neutralino")
#elif defined(__linux__) || defined(__FreeBSD__)
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
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

#if defined(__linux__) || defined(__FreeBSD__)
#define SOCKET_NAME "/tmp/neutralino"
static int socketFD = -1;
static bool isDaemon = false;
static bool run = true;
static int singleInstanceStatus = -1;

static void __socketClose(void) {
    if (socketFD >= 0) {
        if (isDaemon) {
            if (unlink(SOCKET_NAME) < 0)
                debug::log(debug::LogTypeError, "Could not remove FIFO.");
        } else
            close(socketFD);
    }
}

/* returns
 *   -1 on errors
 *    0 on successful server bindings
 *   1 on successful client connects
 */
int __singleInstanceConnect(const char *name) {
    int len, tmpd;
    struct sockaddr_un addr = {0};

    if ((tmpd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        debug::log(debug::LogTypeError, "Could not create socket: ");
        debug::log(debug::LogTypeError, strerror(errno));
        return -1;
    }

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, name);
    len = offsetof(struct sockaddr_un, sun_path) + strlen(name);

    int ret;
    unsigned int retries = 1;
    do {
        ret = bind(tmpd, (struct sockaddr *)&addr, len);
        if (ret == 0) {
            socketFD = tmpd;
            isDaemon = true;
            return 0;
        } else {
            if (errno == EADDRINUSE) {
                ret = connect(tmpd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un));
                if (ret != 0) {
                    if (errno == ECONNREFUSED) {
                        debug::log(debug::LogTypeError, "Could not connect to socket - assuming daemon died.");
                        unlink(name);
                        continue;
                    }
                    debug::log(debug::LogTypeError, "Could not connect to socket: ");
                    debug::log(debug::LogTypeError, strerror(errno));
                    continue;
                }
                debug::log(debug::LogTypeError, "Daemon is already running");
                socketFD = tmpd;
                return 1;
            }
            debug::log(debug::LogTypeError, "Could not bind to socket: ");
            debug::log(debug::LogTypeError, strerror(errno));
            continue;
        }
    } while (retries-- > 0);

    debug::log(debug::LogTypeError, "Could neither connect to an existing daemon nor become one.");
    close(tmpd);
    return -1;
}
#endif

bool __checkSingleInstance() {
    #if defined(_WIN32)
    CreateMutexA(NULL, TRUE, "neutralino_mutex_lock");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return false;
    }
    return true;
    #elif defined(__linux__) || defined(__FreeBSD__)
    singleInstanceStatus = __singleInstanceConnect(SOCKET_NAME);
    return singleInstanceStatus == 0;
    #endif
    //TODO macOS
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
    #elif defined(__linux__) || defined(__FreeBSD__)
    while(true)
    {
        std::vector<uint8_t> dataBuff;
        dataBuff.resize(2048, 0x00);
        size_t dbSize = recv(socketFD, &(dataBuff[0]), dataBuff.size(), 0);

        if(dbSize < 0)
        {
            debug::log(debug::LogTypeError, "No data received");
            break;
        }

        string bufferString(dataBuff.begin(), dataBuff.end());
        json bufferJson = bufferString;
        events::dispatch("otherInstance", bufferJson);
    }
    __socketClose();
    #endif
    //TODO macOS
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
    #elif defined(__linux__) || defined(__FreeBSD__)
    std::string buffer = args.dump();
    size_t buffSize = send(socket_fd, buffer.c_str(), buffer.size(), MSG_CONFIRM);

    if(buffSize != buffer.size())
    {
        debug::log(debug::LogTypeError, "Cannot send data.");
        __socketClose();
        return;
    }

    __socketClose();
    #endif
    //TODO macOS
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
