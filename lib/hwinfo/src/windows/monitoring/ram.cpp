#include "hwinfo/platform.h"

#ifdef HWINFO_WINDOWS

#include <windows.h>

#include "hwinfo/monitoring/ram.h"

namespace hwinfo::monitoring::ram {

Data fetch() {
  MEMORYSTATUSEX status;
  status.dwLength = sizeof(status);
  if (!GlobalMemoryStatusEx(&status)) return {};
  // ullAvailPhys is memory available to the calling process (free + standby).
  // Windows does not expose raw "free" separately via this API, so both fields
  // report the same value (consistent with Memory::free() / Memory::available()).
  return Data{status.ullAvailPhys, status.ullAvailPhys};
}

uint64_t free_bytes() { return fetch().free_bytes; }
uint64_t available_bytes() { return fetch().available_bytes; }

}  // namespace hwinfo::monitoring::ram

#endif  // HWINFO_WINDOWS
