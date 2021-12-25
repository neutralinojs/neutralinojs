#include <vector>
#include <string>
#include <sstream>
#include <time.h>
#include <ctype.h>

#include "helpers.h"
#include "lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace helpers {
    vector<string> split(const string &s, char delim) {
        stringstream ss(s);
        string item;
        vector<string> tokens;
        while (getline(ss, item, delim)) {
            tokens.push_back(item);
        }
        return tokens;
    }

    string generateToken() {
        srand(time(NULL));

        string s = "";
        static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

        for (int i = 0; i < 32; ++i) {
            s += alphanum[rand() % (sizeof(alphanum) - 1)];
        }

        return s;
    }

    /*
    * https://stackoverflow.com/a/14530993 - mini url decoder
    */
    void urldecode(char *dst, const char *src) {
        char a, b;
        while (*src) {
            if ((*src == '%') &&
                ((a = src[1]) && (b = src[2])) &&
                (isxdigit(a) && isxdigit(b))) {
                if (a >= 'a')
                    a -= 'a' - 'A';
                if (a >= 'A')
                    a -= ('A' - 10);
                else
                    a -= '0';
                if (b >= 'a')
                    b -= 'a' - 'A';
                if (b >= 'A')
                    b -= ('A' - 10);
                else
                    b -= '0';
                *dst++ = 16 * a + b;
                src += 3;
            }
            else if (*src == '+') {
                *dst++ = ' ';
                src++;
            }
            else {
                *dst++ = *src++;
            }
        }
        *dst++ = '\0';
    }

    char* cStrCopy(const string &str) {
        char *text = new char[str.size() + 1];
        copy(str.begin(), str.end(), text);
        text[str.size()] = '\0';
        // delete[] text from the initiator
        return text;
    }

    json makeMissingArgErrorPayload() {
        return helpers::makeErrorPayload("NE_RT_NATRTER", "Missing mandatory arguments");
    }

    json makeErrorPayload(const string &code, const string &message) {
        json error;
        error["code"] = code;
        error["message"] = message;
        return error;
    }

    bool hasRequiredFields(const json &input, const vector<string> &keys) {
        for(const string &key: keys) {
            if(!helpers::hasField(input, key)) {
                return false;
            }
        }
        return true;
    }

    bool hasField(const json &input, const string &key) {
        return input.contains(key) && !input[key].is_null();
    }

    /*
    * Thanks to https://github.com/zserge/lorca/blob/a990beb2828ffa625b74500b4ae43b90fdf8b2e1/ui.go#L29
    */
    string getDefaultChromeArgs() {
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
        "--enable-automation "
        "--password-store=basic "
        "--use-mock-keychain";
    }
} // namespace helpers
