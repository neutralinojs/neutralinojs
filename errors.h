#ifndef NEU_ERRORS_H
#define NEU_ERRORS_H

#include <string>

#include "lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace errors {

enum StatusCode {
    // no-error
    NE_ST_OK,
    // storage
    NE_ST_INVSTKY,
    NE_ST_NOSTKEX,
    NE_ST_STKEYWE,
    NE_ST_STKEYRE,
    NE_ST_NOSTDIR,
    // os
    NE_OS_UNLTOUP,
    NE_OS_INVNOTA,
    NE_OS_INVMSGA,
    NE_OS_TRAYIER,
    NE_OS_INVKNPT,
    // extensions
    NE_EX_EXTNOTC,
    // filesystem
    NE_FS_FILWRER,
    NE_FS_DIRCRER,
    NE_FS_REMVERR,
    NE_FS_FILRDER,
    NE_FS_NOPATHE,
    NE_FS_NOTADIR,
    NE_FS_COPYERR,
    NE_FS_MOVEERR,
    NE_FS_FILOPER,
    NE_FS_UNLTOUP,
    NE_FS_UNLTFOP,
    NE_FS_UNLCWAT,
    NE_FS_NOWATID,
    NE_FS_UNLSTPR,
    // window
    NE_WI_UNBSWSR,
    // router
    NE_RT_INVTOKN,
    NE_RT_APIPRME,
    NE_RT_NATPRME,
    NE_RT_NATRTER,
    NE_RT_NATNTIM,
    // resources
    NE_RS_TREEGER,
    NE_RS_UNBLDRE,
    NE_RS_NOPATHE,
    NE_RS_FILEXTF,
    NE_RS_DIREXTF,
    // server
    NE_SR_UNBSEND,
    NE_SR_UNBPARS,
    NE_SR_MPINUSE,
    NE_SR_NOMTPTH,
    // config
    NE_CF_UNBLDCF,
    NE_CF_UNBPRCF,
    NE_CF_UNSUPMD,
    NE_CF_UNBLWCF
};

json makeMissingArgErrorPayload(const string& missingArg);
json makeErrorPayload(const errors::StatusCode code, const string &param = "");
string makeErrorMsg(const errors::StatusCode code, const string &param = "");

} // namespace errors

#endif // #define NEU_ERRORS_H


