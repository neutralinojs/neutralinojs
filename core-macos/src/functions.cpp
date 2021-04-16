#include <iostream>
#include <sstream>
#include <vector>
#include <time.h>

#ifdef __APPLE__
#include "CoreFoundation/CoreFoundation.h"
#endif

using namespace std;
namespace functions {

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
        srand (time(NULL));

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

    string getAppPath() {
        // Latest MACOS requires to obtain a full path to resources using Bundle API
        #ifdef __APPLE__
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
        char path[PATH_MAX];
        if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
        {
            // error!
        }
        CFRelease(resourcesURL);

        std::string ppath(path);
        return ppath;
        #endif
        return "";
    }

}
