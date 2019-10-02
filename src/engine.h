#ifndef ENGINE_H
#define ENGINE_H

#include <string>

#include "webview.h"
#include "io/fs/NFile.h"

using namespace std;

namespace neut {
class Engine {
private:
  static string _resourceRoot;
public:
  Engine();
  ~Engine();
  static string getResourceRoot();
};
} // namespace neut
#endif