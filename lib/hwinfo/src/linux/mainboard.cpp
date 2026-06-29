// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include "hwinfo/platform.h"

#ifdef HWINFO_UNIX

#include <fstream>
#include <vector>

#include "hwinfo/mainboard.h"

namespace hwinfo {

std::string get_dmi_by_name(const std::string& name) {
  std::string value;
  std::vector<std::string> candidates = {"/sys/devices/virtual/dmi/", "/sys/class/dmi/"};
  for (const auto& path : candidates) {
    std::string full_path(path);
    full_path.append("id/");
    full_path.append(name);
    std::ifstream f(full_path);
    if (f) {
      getline(f, value);
      if (!value.empty()) {
        return value;
      }
    }
  }
  return "<unknown>";
}

// _____________________________________________________________________________________________________________________
MainBoard::MainBoard() {
  _vendor = get_dmi_by_name("board_vendor");
  _name = get_dmi_by_name("board_name");
  _version = get_dmi_by_name("board_version");
  _serial_number = get_dmi_by_name("board_serial");
}

}  // namespace hwinfo

#endif  // HWINFO_UNIX
