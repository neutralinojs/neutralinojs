#include "hwinfo/platform.h"

#ifdef HWINFO_WINDOWS

#include <windows.h>

#include "hwinfo/monitoring/disk.h"

namespace hwinfo::monitoring::disk {

std::uint64_t get_free_size(const std::string& mount_point) {
  ULARGE_INTEGER freeBytesAvailableToCaller;
  if (GetDiskFreeSpaceExA(mount_point.c_str(), &freeBytesAvailableToCaller, nullptr, nullptr)) {
    return freeBytesAvailableToCaller.QuadPart;
  }
  return 0;
}

std::uint64_t get_free_size(const Disk& disk) {
  std::uint64_t size = 0;
  for (const auto& mount_point : disk.mount_points()) {
    size += get_free_size(mount_point);
  }
  return size;
}

Data fetch(const std::string& mount_point) {
  return Data{mount_point, get_free_size(mount_point)};
}

}  // namespace hwinfo::monitoring::disk

#endif  // HWINFO_WINDOWS
