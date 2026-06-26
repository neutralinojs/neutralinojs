#pragma once

#include <cstdint>
#include <string>

#include "hwinfo/disk.h"
#include "hwinfo/monitoring/monitor.h"

namespace hwinfo::monitoring::disk {

struct Data {
  std::string mount_point;
  uint64_t free_bytes;
};

// Returns free bytes for the given mount point path.
std::uint64_t get_free_size(const std::string& mount_point);

// Returns the sum of free bytes across all mount points of the given disk.
std::uint64_t get_free_size(const Disk& disk);

// Fetches a Data snapshot for a single mount point.
Data fetch(const std::string& mount_point);

using Monitor = hwinfo::monitoring::Monitor<Data>;

}  // namespace hwinfo::monitoring::disk
