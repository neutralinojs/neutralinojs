#include <array>
#include <string>
#include <libgen.h>
#include <stdio.h>
#include <unistd.h>
#include "lib/easylogging/easylogging++.h"

using namespace std;

namespace platform {
    string getDirectoryName(string filename){
        return dirname((char*)filename.c_str());
    }

    string execCommand(string command){
        std::array<char, 128> buffer;
        std::string result = "";
        std::shared_ptr<FILE> pipe(popen((command + " 2>&1").c_str(), "r"), pclose);
        if (!pipe) {
            LOG(ERROR) << "Pipe open failed.";
        }
        else {
            while (!feof(pipe.get())) {
                if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
                    result += buffer.data();
            }
        }
        return result;
    }
    
    string getCurrentDirectory() {
        return getcwd(NULL, 0);
    }
}
