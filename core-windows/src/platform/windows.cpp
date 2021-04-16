#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <windows.h>
#include <shlwapi.h>
#include "helpers.h"

using namespace std;

namespace windows {
    string getDirectoryName(string filename){
        LPSTR pathToReplace = const_cast<char *>(filename.c_str());
        PathRemoveFileSpecA(pathToReplace);
        std::string directory(pathToReplace);
        std::replace(directory.begin(), directory.end(), '\\', '/');
        return directory;
    }

    string execCommand(string command) {
        string output = "";
        PROCESS_INFORMATION pi;
        STARTUPINFO si = {sizeof(si)};
        char temp[256];
        GetTempPathA(256, temp);
        string tmpFile = string(temp) + "nl_o" + helpers::generateToken() + ".tmp";
        CreateProcessA(NULL,(LPSTR)(command + " > " + tmpFile).c_str(),NULL,NULL,TRUE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi);
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        ifstream t;
        t.open(tmpFile);
        if(!t.is_open())
            return "";
        string buffer = "";
        string line;
        while(!t.eof()){
            getline(t, line);
            buffer += line + "\n";
        }
        t.close();
        output = buffer;
        DeleteFile(tmpFile.c_str());
        return output;
    }
}
