// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include "hwinfo/platform.h"

#ifdef HWINFO_UNIX

#include <sys/stat.h>
#include <sys/utsname.h>

#include <fstream>
#include <string>

#include "hwinfo/os.h"
#include "hwinfo/utils/stringutils.h"

namespace hwinfo {

// _____________________________________________________________________________________________________________________
OS::OS() {
  {  // name and version
    std::string line;
    std::ifstream stream("/etc/os-release");
    if (!stream) {
      _name = "Linux";
      _version = "<unknown>";
    }
    while (std::getline(stream, line)) {
      if (utils::starts_with(line, "PRETTY_NAME")) {
        line = line.substr(line.find('=') + 1, line.length());
        // remove \" at begin and end of the substring result
        _name = {line.begin() + 1, line.end() - 1};
      }
      if (utils::starts_with(line, "VERSION=")) {
        line = line.substr(line.find('=') + 1, line.length());
        // remove \" at begin and end of the substring result
        _version = {line.begin() + 1, line.end() - 1};
      }
    }
    stream.close();
  }
  {  // Kernel
    static utsname info;
    if (uname(&info) == 0) {
      _kernel = info.release;
    } else {
      _kernel = "<unknown>";
    }
  }
  {  // architecture
    struct stat buffer {};
    _64bit = stat("/lib64/ld-linux-x86-64.so.2", &buffer) == 0;
    _32bit = !_64bit;
  }
  {  // Get endian. This is platform independent...
    char16_t dummy = 0x0102;
    _bigEndian = ((char*)&dummy)[0] == 0x01;
    _littleEndian = ((char*)&dummy)[0] == 0x02;
  }
}

}  // namespace hwinfo

#endif  // HWINFO_UNIX
