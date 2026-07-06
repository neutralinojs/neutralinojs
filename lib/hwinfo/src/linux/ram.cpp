// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include "hwinfo/platform.h"

#ifdef HWINFO_UNIX

#include <dlfcn.h>
#include <hwinfo/ram.h>
#include <hwinfo/utils/stringutils.h>
#include <hwinfo/utils/utils.h>
#include <unistd.h>

#include <fstream>
#include <string>
#include <vector>

namespace hwinfo {

// _____________________________________________________________________________________________________________________
struct MemInfo {
  uint64_t total = 0;
  uint64_t free = 0;
  uint64_t available = 0;
};

void get_from_sysconf(MemInfo& mi) {
  int64_t pages = sysconf(_SC_PHYS_PAGES);
  int64_t available_pages = sysconf(_SC_AVPHYS_PAGES);
  int64_t page_size = sysconf(_SC_PAGESIZE);
  if (pages > 0 && page_size > 0) {
    mi.total = pages * page_size;
  }
  if (available_pages > 0 && page_size > 0) {
    mi.available = available_pages * page_size;
  }
}

void set_value(std::string& line, uint64_t* dst) {
  auto split_line = utils::split(line, ":");
  if (split_line.size() == 2) {
    auto& value = split_line[1];
    utils::strip(value);
    auto space = value.find(' ');
    if (space != std::string::npos) {
      auto a = std::string(value.begin(), value.begin() + static_cast<int64_t>(space));
      *dst = (std::stoull(a) * 1024);
    }
  }
}

MemInfo parse_meminfo() {
  MemInfo mi;
  std::ifstream f_meminfo("/proc/meminfo");
  if (!f_meminfo) {
    get_from_sysconf(mi);
  } else {
    while (mi.total == 0 || mi.available == 0 || mi.free == 0) {
      std::string line;
      if (!std::getline(f_meminfo, line)) {
        if (mi.total == 0 || mi.available == 0) {
          get_from_sysconf(mi);
        }
        return mi;
      }
      auto split = utils::split(line, ":");
      if (split.size() != 2) {
        continue;
      }
      auto key = std::move(split[0]);
      auto value = std::move(split[1]);
      utils::strip(key);
      utils::strip(value);
      auto val_split = utils::split(value, " ");
      std::uint64_t val = 0;
      if (!val_split.empty()) {
        try {
          val = std::stoull(val_split[0]) * 1024;
        } catch (...) {
        }
      }
      if (key == "MemTotal") {
        mi.total = val;
      } else if (key == "MemFree") {
        mi.free = val;
      } else if (key == "MemAvailable") {
        mi.available = val;
      }
    }
  }
  return mi;
}

// _____________________________________________________________________________________________________________________
Memory::Memory() {
  // TODO: get information for actual memory modules (DIMM)
}

// _____________________________________________________________________________________________________________________
uint64_t Memory::size() const {
  auto meminfo = parse_meminfo();
  return utils::round_to_next_power_of_2(meminfo.total);
}

// _____________________________________________________________________________________________________________________
uint64_t Memory::free() const {
  auto meminfo = parse_meminfo();
  return meminfo.free;
}

// _____________________________________________________________________________________________________________________
uint64_t Memory::available() const {
  auto meminfo = parse_meminfo();
  return meminfo.available;
}

}  // namespace hwinfo

#endif  // HWINFO_UNIX
