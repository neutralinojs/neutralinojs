#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <windows.h>
#include <atlstr.h>
#include <shlwapi.h>
#include "helpers.h"

#define EXEC_BUFSIZE 4096

using namespace std;

namespace platform {
    string getDirectoryName(string filename){
        LPSTR pathToReplace = const_cast<char *>(filename.c_str());
        PathRemoveFileSpecA(pathToReplace);
        std::string directory(pathToReplace);
        std::replace(directory.begin(), directory.end(), '\\', '/');
        return directory;
    }

    // A modified version of https://stackoverflow.com/a/59523254
    string execCommand(string command) {
        string output = "";
        HANDLE g_hChildStd_OUT_Rd = NULL;
        HANDLE g_hChildStd_OUT_Wr = NULL;

        SECURITY_ATTRIBUTES sa;
        // Set the bInheritHandle flag so pipe handles are inherited.
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;
        if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &sa, 0))     { return output; } // Create a pipe for the child process's STDOUT.
        if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) { return output; } // Ensure the read handle to the pipe for STDOUT is not inherited

        PROCESS_INFORMATION piProcInfo;
        STARTUPINFO siStartInfo;
        bool bSuccess = FALSE;

        // Set up members of the PROCESS_INFORMATION structure.
        ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

        // Set up members of the STARTUPINFO structure.
        // This structure specifies the STDERR and STDOUT handles for redirection.
        ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
        siStartInfo.cb = sizeof(STARTUPINFO);
        siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

        // Create the child process.
        bSuccess = CreateProcess(
            NULL,             // program name
            (LPSTR)(command + " 2>&1").c_str(),       // command line
            NULL,             // process security attributes
            NULL,             // primary thread security attributes
            TRUE,             // handles are inherited
            CREATE_NO_WINDOW, // creation flags (this is what hides the window)
            NULL,             // use parent's environment
            NULL,             // use parent's current directory
            &siStartInfo,     // STARTUPINFO pointer
            &piProcInfo       // receives PROCESS_INFORMATION
        );
        CloseHandle(g_hChildStd_OUT_Wr);

        // read output
        DWORD dwRead;
        CHAR chBuf[EXEC_BUFSIZE];
        bool bSuccess2 = FALSE;
        for (;;) { // read stdout
            bSuccess2 = ReadFile(g_hChildStd_OUT_Rd, chBuf, EXEC_BUFSIZE, &dwRead, NULL);
            if(!bSuccess2 || dwRead == 0) break;
            std::string s(chBuf, dwRead);
            output += s;
        }

        // The remaining open handles are cleaned up when this process terminates.
        // To avoid resource leaks in a larger application,
        // close handles explicitly.
        return output;
    }

    string getCurrentDirectory() {
        TCHAR currentDir[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, currentDir);   
        std::string currentDirStr(currentDir);
        std::replace(currentDirStr.begin(), currentDirStr.end(), '\\', '/');     
        return currentDirStr;
    }
}
