#ifndef NEU_PERMISSION_H
#define NEU_PERMISSION_H

#include <string>

using namespace std;

namespace permission {

enum FileSystemAccessPermission { FileSystemAccessPermissionRead, FileSystemAccessPermissionWrite };

void init();
bool hasMethodAccess(const string &func);
bool hasAPIAccess();
bool hasCommandExecutionAccess(const string &command);
bool hasFileSystemPathAccess(const string &path, permission::FileSystemAccessPermission perm);

} // namespace permission

#endif // #define NEU_PERMISSION_H
