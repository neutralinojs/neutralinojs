#include <string>
#include <vector>
#include <regex>

#include "lib/json/json.hpp"
#include "lib/filedialogs/portable-file-dialogs.h"
#include "settings.h"
#include "helpers.h"
#include "api/os/os.h"
#include "api/fs/fs.h"

using namespace std;
using json = nlohmann::json;

/*
* C++ rewrite of https://github.com/zserge/lorca/ (Go-based) concept
*/

namespace chrome {

string __getDefaultChromeArgs() {
    return "--disable-background-networking "
    "--disable-background-timer-throttling "
    "--disable-backgrounding-occluded-windows "
    "--disable-breakpad "
    "--disable-client-side-phishing-detection "
    "--disable-default-apps "
    "--disable-dev-shm-usage "
    "--disable-infobars "
    "--disable-extensions "
    "--disable-features=site-per-process "
    "--disable-hang-monitor "
    "--disable-ipc-flooding-protection "
    "--disable-popup-blocking "
    "--disable-prompt-on-repost "
    "--disable-renderer-backgrounding "
    "--disable-sync "
    "--disable-translate "
    "--disable-windows10-custom-titlebar "
    "--metrics-recording-only "
    "--no-first-run "
    "--no-default-browser-check "
    "--safebrowsing-disable-auto-update "
    "--password-store=basic "
    "--use-mock-keychain";
}

string __findChrome(const json &input) {
    string chromePath = "";
    #if defined(__linux__)
    vector<string> chromeBins = {
        "/usr/bin/google-chrome-stable",
        "/usr/bin/google-chrome",
        "/usr/bin/chromium",
        "/usr/bin/chromium-browser",
        "/snap/bin/chromium",
    };

    #elif defined(__APPLE__)
    vector<string> chromeBins = {
        "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome",
        "/Applications/Google Chrome Canary.app/Contents/MacOS/Google Chrome Canary",
        "/Applications/Chromium.app/Contents/MacOS/Chromium",
        "/Applications/Microsoft Edge.app/Contents/MacOS/Microsoft Edge",
        "/usr/bin/google-chrome-stable",
        "/usr/bin/google-chrome",
        "/usr/bin/chromium",
        "/usr/bin/chromium-browser",
    };

    #elif defined(_WIN32)
    vector<string> chromeBins = {
        os::getEnv("LocalAppData") + "/Google/Chrome/Application/chrome.exe",
        os::getEnv("ProgramFiles") + "/Google/Chrome/Application/chrome.exe",
        os::getEnv("ProgramFiles(x86)") + "/Google/Chrome/Application/chrome.exe",
        os::getEnv("LocalAppData") + "/Chromium/Application/chrome.exe",
        os::getEnv("ProgramFiles") + "/Chromium/Application/chrome.exe",
        os::getEnv("ProgramFiles(x86)") + "/Chromium/Application/chrome.exe",
        os::getEnv("ProgramFiles(x86)") + "/Microsoft/Edge/Application/msedge.exe",
        os::getEnv("ProgramFiles") + "/Microsoft/Edge/Application/msedge.exe",
    };
    #endif

    string browserBinaryKeyForOs = "browserBinary" + string(NEU_OS_NAME);
    if(helpers::hasField(input, "browserBinary") || helpers::hasField(input, browserBinaryKeyForOs)) {
        string customBin = helpers::hasField(input, browserBinaryKeyForOs) ? input[browserBinaryKeyForOs].get<string>() : 
                                    input["browserBinary"].get<string>();
        chromeBins.insert(chromeBins.begin(), fs::applyPathConstants(customBin));
    }

    for(const string &cmd: chromeBins) {
        fs::FileStats stats = fs::getStats(cmd);
        if(stats.status == errors::NE_ST_OK && stats.entryType == fs::EntryTypeFile) {
            chromePath = cmd;
            break;
        }
    }
    return chromePath;
}

void init(const json &input) {

    string chromeCmd = __findChrome(input);

    if(chromeCmd.empty()) {
        pfd::message("Unable to start Chrome mode",
                        "You need to install Google Chrome to run this Neutralinojs application",
                        pfd::choice::ok,
                        pfd::icon::error);
        std::exit(1);
    }

    if(regex_search(chromeCmd, regex(" "))) {
        chromeCmd = "\"" + chromeCmd + "\"";
    }

    chromeCmd += " " + __getDefaultChromeArgs();

    chromeCmd += " --user-data-dir=\"" + settings::joinAppDataPath("/.tmp/chromedata") + "\"";
    chromeCmd += " --app=\"" + input["url"].get<string>() + "\"";

    if(helpers::hasRequiredFields(input, {"width", "height"})) {
        chromeCmd += " --window-size=" + to_string(input["width"].get<int>()) +
                    "," + to_string(input["height"].get<int>());
    }

    if(helpers::hasField(input, "args")) {
        chromeCmd += " " + input["args"].get<string>();
    }

    os::ChildProcessOptions processOptions;
    processOptions.background = true;
    
    os::execCommand(chromeCmd, processOptions);
}

} // namespace chrome

