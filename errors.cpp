#include <string>
#include <regex>

#include "errors.h"
#include "lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace errors {

string __getStatusCodeString(const errors::StatusCode code) {
    switch(code) {
        // no-error
        case errors::NE_ST_OK: return "NE_ST_OK";
        // storage
        case errors::NE_ST_INVSTKY: return "NE_ST_INVSTKY";
        case errors::NE_ST_NOSTKEX: return "NE_ST_NOSTKEX";
        case errors::NE_ST_STKEYWE: return "NE_ST_STKEYWE";
        case errors::NE_ST_STKEYRE: return "NE_ST_STKEYRE";
        case errors::NE_ST_NOSTDIR: return "NE_ST_NOSTDIR";
        // os
        case errors::NE_OS_UNLTOUP: return "NE_OS_UNLTOUP";
        case errors::NE_OS_INVNOTA: return "NE_OS_INVNOTA";
        case errors::NE_OS_INVMSGA: return "NE_OS_INVMSGA";
        case errors::NE_OS_TRAYIER: return "NE_OS_TRAYIER";
        case errors::NE_OS_INVKNPT: return "NE_OS_INVKNPT";
        // extensions
        case errors::NE_EX_EXTNOTC: return "NE_EX_EXTNOTC";
        // filesystem
        case errors::NE_FS_FILWRER: return "NE_FS_FILWRER";
        case errors::NE_FS_DIRCRER: return "NE_FS_DIRCRER";
        case errors::NE_FS_REMVERR: return "NE_FS_REMVERR";
        case errors::NE_FS_FILRDER: return "NE_FS_FILRDER";
        case errors::NE_FS_NOPATHE: return "NE_FS_NOPATHE";
        case errors::NE_FS_NOTADIR: return "NE_FS_NOTADIR";
        case errors::NE_FS_COPYERR: return "NE_FS_COPYERR";
        case errors::NE_FS_MOVEERR: return "NE_FS_MOVEERR";
        case errors::NE_FS_FILOPER: return "NE_FS_FILOPER";
        case errors::NE_FS_UNLTOUP: return "NE_FS_UNLTOUP";
        case errors::NE_FS_UNLTFOP: return "NE_FS_UNLTFOP";
        case errors::NE_FS_UNLCWAT: return "NE_FS_UNLCWAT";
        case errors::NE_FS_NOWATID: return "NE_FS_NOWATID";
        case errors::NE_FS_UNLSTPR: return "NE_FS_UNLSTPR";
        // window
        case errors::NE_WI_UNBSWSR: return "NE_WI_UNBSWSR";
        // router
        case errors::NE_RT_INVTOKN: return "NE_RT_INVTOKN";
        case errors::NE_RT_APIPRME: return "NE_RT_APIPRME";
        case errors::NE_RT_NATPRME: return "NE_RT_NATPRME";
        case errors::NE_RT_NATRTER: return "NE_RT_NATRTER";
        case errors::NE_RT_NATNTIM: return "NE_RT_NATNTIM";
        // resources
        case errors::NE_RS_TREEGER: return "NE_RS_TREEGER";
        case errors::NE_RS_UNBLDRE: return "NE_RS_UNBLDRE";
        case errors::NE_RS_NOPATHE: return "NE_RS_NOPATHE";
        case errors::NE_RS_FILEXTF: return "NE_RS_FILEXTF";
        case errors::NE_RS_DIREXTF: return "NE_RS_DIREXTF";
        // server
        case errors::NE_SR_UNBSEND: return "NE_SR_UNBSEND";
        case errors::NE_SR_UNBPARS: return "NE_SR_UNBPARS";
        case errors::NE_SR_MPINUSE: return "NE_SR_MPINUSE";
        case errors::NE_SR_NOMTPTH: return "NE_SR_NOMTPTH";
        // config
        case errors::NE_CF_UNBLDCF: return "NE_CF_UNBLDCF";
        case errors::NE_CF_UNBPRCF: return "NE_CF_UNBPRCF";
        case errors::NE_CF_UNSUPMD: return "NE_CF_UNSUPMD";
        case errors::NE_CF_UNBLWCF: return "NE_CF_UNBLWCF";
    }
    return "NE_ST_NOTOK";
}

string __findStatusCodeDesc(errors::StatusCode code) {
    switch(code) {
        // no-error
        case errors::NE_ST_OK: return "";
        // storage
        case errors::NE_ST_INVSTKY: return "Invalid storage key format. The key should match regex: %1";
        case errors::NE_ST_NOSTKEX: return "Unable to find storage key: %1";
        case errors::NE_ST_STKEYWE: return "Unable to write data to key: %1";
        case errors::NE_ST_STKEYRE: return "Unable to remove storage key: %1";
        case errors::NE_ST_NOSTDIR: return "Unable to read storage directory: %1";
        // os
        case errors::NE_OS_UNLTOUP: return "Unable to update process id: %1";
        case errors::NE_OS_INVNOTA: return "Invalid notification style arguments: %1";
        case errors::NE_OS_INVMSGA: return "Invalid message box style arguments: %1";
        case errors::NE_OS_TRAYIER: return "Unable to initialize the tray menu";
        case errors::NE_OS_INVKNPT: return "Invalid platform path name: %1";
        // extensions
        case errors::NE_EX_EXTNOTC: return "%1 is not connected yet";
        // filesystem
        case errors::NE_FS_FILWRER: return "Unable to write file: %1";
        case errors::NE_FS_DIRCRER: return "Cannot create a directory in %1";
        case errors::NE_FS_REMVERR: return "Cannot remove path: %1";
        case errors::NE_FS_FILRDER: return "Unable to open file: %1";
        case errors::NE_FS_NOPATHE: return "Unable to open path %1";
        case errors::NE_FS_NOTADIR: return "Path %1 is not a directory";
        case errors::NE_FS_COPYERR: return "Cannot perform copy: %1";
        case errors::NE_FS_MOVEERR: return "Cannot perform move: %1";
        case errors::NE_FS_FILOPER: return "Unable to open file: %1";
        case errors::NE_FS_UNLTOUP: return "Unable to update opened file id: %1";
        case errors::NE_FS_UNLTFOP: return "Unable to find opened file id: %1";
        case errors::NE_FS_UNLCWAT: return "Unable to create watcher for path: %1";
        case errors::NE_FS_NOWATID: return "Unable to find watcher: %1";
        case errors::NE_FS_UNLSTPR: return "Unable to set file permissions for %1";
        // window
        case errors::NE_WI_UNBSWSR: return "Unable to save window screenshot to %1";
        // router
        case errors::NE_RT_INVTOKN: return "Invalid or expired NL_TOKEN value from client";
        case errors::NE_RT_APIPRME: return "Missing permission to access Native API";
        case errors::NE_RT_NATPRME: return "Missing permission to execute the native method: %1";
        case errors::NE_RT_NATRTER: return "Native method execution error occurred. Required parameter is missing: %1";
        case errors::NE_RT_NATNTIM: return "%1 is not implemented in the Neutralinojs server";
        // resources
        case errors::NE_RS_TREEGER: return "Resource file tree generation error. %1 is missing.";
        case errors::NE_RS_UNBLDRE: return "Unable to load application resource file %1";
        case errors::NE_RS_NOPATHE: return "Path (%1) doesn't exist in resources";
        case errors::NE_RS_FILEXTF: return "Unable to extract the requested file to %1";
        case errors::NE_RS_DIREXTF: return "Unable to extract the requested directory to %1";
        // server
        case errors::NE_SR_UNBSEND: return "Unable to send native message";
        case errors::NE_SR_UNBPARS: return "Unable to parse native call payload";
        case errors::NE_SR_MPINUSE: return "Mount path is already in use: %1";
        case errors::NE_SR_NOMTPTH: return "Cannot unmount %1, the path that was not mounted";
        // config
        case errors::NE_CF_UNBLDCF: return "Unable to load the %1 configuration file. Framework defaults will be loaded.";
        case errors::NE_CF_UNBPRCF: return "Unable to parse the config file: %1";
        case errors::NE_CF_UNSUPMD: return "Unsupported mode: %1. The default mode (window) is selected.";
                case errors::NE_CF_UNBLWCF: return "Unable to load the window config file: %1";
    }
    return "";
}

string __getStatusCodeDesc(errors::StatusCode code, string param = "") {
    string msg = __findStatusCodeDesc(code);
    return regex_replace(msg, regex("%1"), param);
}

json makeMissingArgErrorPayload(const string& missingArg) {
    return errors::makeErrorPayload(errors::NE_RT_NATRTER, missingArg);
}

json makeErrorPayload(const errors::StatusCode code, const string &param) {
    json error;
    error["code"] = __getStatusCodeString(code);
    error["message"] = __getStatusCodeDesc(code, param);
    return error;
}

string makeErrorMsg(const errors::StatusCode code, const string &param) {
    return __getStatusCodeString(code) + ": " + __getStatusCodeDesc(code, param);
}

} // namespace errors
