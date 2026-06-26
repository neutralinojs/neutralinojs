#include "hwinfo/platform.h"

#ifdef HWINFO_UNIX

#include <fstream>
#include <string>

#include "hwinfo/monitoring/ram.h"

namespace {

hwinfo::monitoring::ram::Data read_meminfo() {
  hwinfo::monitoring::ram::Data d{};
  std::ifstream f("/proc/meminfo");
  if (!f.is_open()) return d;
  std::string key;
  uint64_t val = 0;
  std::string unit;
  while (f >> key >> val) {
    std::getline(f, unit);  // consume remainder of line (e.g. " kB\n")
    const uint64_t bytes = val * 1024;
    if (key == "MemFree:") {
      d.free_bytes = bytes;
    } else if (key == "MemAvailable:") {
      d.available_bytes = bytes;
    }
    if (d.free_bytes && d.available_bytes) break;
  }
  return d;
}

}  // namespace

namespace hwinfo::monitoring::ram {

Data fetch() { return read_meminfo(); }
uint64_t free_bytes() { return read_meminfo().free_bytes; }
uint64_t available_bytes() { return read_meminfo().available_bytes; }

}  // namespace hwinfo::monitoring::ram

#endif  // HWINFO_UNIX
