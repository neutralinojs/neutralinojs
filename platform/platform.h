#if defined(__APPLE__)
#include <objc/objc-runtime.h>

// Helpers to avoid too much typing with the Objective c runtime.
inline id operator"" _cls(const char *s, std::size_t) { return (id)objc_getClass(s); }
inline SEL operator"" _sel(const char *s, std::size_t) { return sel_registerName(s); }
inline id operator"" _str(const char *s, std::size_t) {
  return ((id(*)(id, SEL, const char *))objc_msgSend)(
      "NSString"_cls, "stringWithUTF8String:"_sel, s);
}
#endif

using namespace std;

namespace platform {
    string getDirectoryName(string filename);
    string execCommand(string command);
    string getCurrentDirectory();
}
