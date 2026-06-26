#include "hwinfo/platform.h"

#ifdef HWINFO_UNIX

#include <sys/statvfs.h>

#include "hwinfo/monitoring/disk.h"

namespace hwinfo::monitoring::disk {

std::uint64_t get_free_size(const std::string& mount_point) {
  struct statvfs stat {};
  if (statvfs(mount_point.c_str(), &stat) == 0) {
    return static_cast<uint64_t>(stat.f_bavail) * static_cast<uint64_t>(stat.f_frsize);
  }
  return 0;
}

std::uint64_t get_free_size(const Disk& disk) {
  std::uint64_t total = 0;
  for (const auto& mp : disk.mount_points()) {
    total += get_free_size(mp);
  }
  return total;
}

Data fetch(const std::string& mount_point) {
  return Data{mount_point, get_free_size(mount_point)};
}

}  // namespace hwinfo::monitoring::disk

#endif  // HWINFO_UNIX
