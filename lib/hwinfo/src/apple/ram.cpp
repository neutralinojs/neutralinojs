// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include "hwinfo/platform.h"

#ifdef HWINFO_APPLE

#include <hwinfo/ram.h>
#include <mach/mach_host.h>
#include <mach/mach_init.h>
#include <mach/vm_statistics.h>
#include <sys/sysctl.h>

#include <string>

namespace hwinfo {

// _____________________________________________________________________________________________________________________
uint64_t getMemSize() {
  uint64_t memSize;
  size_t size = sizeof(memSize);

  if (sysctlbyname("hw.memsize", &memSize, &size, nullptr, 0) == 0) {
    return memSize;
  }

  return 0;
}

// _____________________________________________________________________________________________________________________
Memory::Memory() {
  // TODO: get information for actual memory modules (DIMM)
}

// _____________________________________________________________________________________________________________________
uint64_t Memory::size() const {
  uint64_t sum = 0;
  for (const auto& module : _modules) {
    sum += module._size_bytes;
  }
  return sum;
}

// _____________________________________________________________________________________________________________________
uint64_t Memory::free() const {
  vm_statistics64_data_t vmStats;
  mach_msg_type_number_t infoCount = HOST_VM_INFO64_COUNT;
  kern_return_t kernReturn =
      host_statistics64(mach_host_self(), HOST_VM_INFO64, reinterpret_cast<host_info64_t>(&vmStats), &infoCount);

  if (kernReturn == KERN_SUCCESS) {
    vm_size_t pageSize;
    host_page_size(mach_host_self(), &pageSize);

    int64_t totalMemory = getMemSize();
    if (totalMemory == -1) return -1;

    const int64_t appMemory =
        static_cast<int64_t>(vmStats.internal_page_count - vmStats.purgeable_count) * static_cast<int64_t>(pageSize);
    const int64_t wiredMemory = static_cast<int64_t>(vmStats.wire_count) * static_cast<int64_t>(pageSize);
    const int64_t compressedMemory =
        static_cast<int64_t>(vmStats.compressor_page_count) * static_cast<int64_t>(pageSize);

    const int64_t usedMemory = appMemory + wiredMemory + compressedMemory;
    return totalMemory - usedMemory;
  }

  return 0;
}

// _____________________________________________________________________________________________________________________
uint64_t Memory::available() const {
  int64_t usableMemSize;
  size_t size = sizeof(usableMemSize);

  if (sysctlbyname("hw.memsize_usable", &usableMemSize, &size, nullptr, 0) == 0) {
    return usableMemSize;
  }

  return 0;
}

}  // namespace hwinfo

#endif  // HWINFO_APPLE