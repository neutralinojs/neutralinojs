#include "engine.h"
#include <limits.h>

#ifdef __linux__
#include <unistd.h>
#endif

#ifdef __APPLE__
#include "CoreFoundation/CoreFoundation.h"
#endif

#ifdef _WIN32
#include <windows.h>
#define PATH_MAX MAX_PATH
#endif

using namespace neut;

Engine::Engine() {}

string Engine::getResourceRoot() {
  if(!_resourceRoot.empty()) { return _resourceRoot; }

  char ppath[PATH_MAX];
  // Latest MACOS requires to obtain a full path to resources using Bundle API
#ifdef __APPLE__
  CFBundleRef mainBundle = CFBundleGetMainBundle();
  CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
  if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)ppath,
                                        PATH_MAX)) {
    // error!
  }
  CFRelease(resourcesURL);
  _resourceRoot = string(ppath) + "/";
#endif

// get executable location in linux
#ifdef __linux__
  size_t sz = readlink("/proc/self/exe", ppath, PATH_MAX);
  ppath[sz + 1] = '\0';
  string realpath(ppath);
  size_t loc = realpath.find_last_of("/");
  _resourceRoot = realpath.substr(0, loc) + "\";
#endif

// get executable location in win32
#ifdef WIN32
  GetModuleFileNameA(NULL, ppath, PATH_MAX);
  string realpath(ppath);
  size_t loc = realpath.find_last_of("\\");
  _resourceRoot = realpath.substr(0, loc) + "\\";
#endif

  return _resourceRoot;
}
