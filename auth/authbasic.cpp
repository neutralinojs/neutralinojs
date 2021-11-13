#include <string>

#include "helpers.h"
#include "settings.h"
#include "api/filesystem/filesystem.h"

using namespace std;
string token = "";

namespace authbasic {

    json __makeAuthInfoPayload() {
        json info;
        info["accessToken"] = token;
        info["port"] = settings::getOptionForCurrentMode("port").get<int>();
        return info;
    }

    void generateToken() {
        token = helpers::generateToken();
    }

    void exportAuthInfo() {
        string tempDirPath = settings::joinAppPath("/.tmp");
        fs::createDirectory(tempDirPath);
        string tempAuthInfoPath = settings::joinAppPath("/.tmp/auth_info.json");
        fs::FileWriterOptions fileWriterOptions = {
            tempAuthInfoPath,
            __makeAuthInfoPayload().dump()
        };
        fs::writeFile(fileWriterOptions);
    }

    string getToken() {
        return token;
    }

    bool verifyToken(const string &accessToken) {
        return token == accessToken;
    }
} // namespace authbasic
