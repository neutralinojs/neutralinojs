#include <objc/objc-runtime.h>

using namespace std;

// Helpers to avoid too much typing with the Objective c runtime.
inline id operator"" _cls(const char *s, std::size_t) { return (id)objc_getClass(s); }
inline SEL operator"" _sel(const char *s, std::size_t) { return sel_registerName(s); }
inline id operator"" _str(const char *s, std::size_t) {
  return ((id(*)(id, SEL, const char *))objc_msgSend)(
      "NSString"_cls, "stringWithUTF8String:"_sel, s);
}

namespace macos {
    string getDirectoryName(string filename);
    string execCommand(string command);
}
