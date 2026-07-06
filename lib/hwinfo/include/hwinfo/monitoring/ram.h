#pragma once

#include <cstdint>

#include "hwinfo/monitoring/monitor.h"

namespace hwinfo::monitoring::ram {

struct Data {
  uint64_t free_bytes;      // memory not in use at all
  uint64_t available_bytes; // memory available for new allocations (includes reclaimable)
};

// Returns free physical memory in bytes.
uint64_t free_bytes();

// Returns available physical memory in bytes.
uint64_t available_bytes();

// Fetches a complete Data snapshot.
Data fetch();

using Monitor = hwinfo::monitoring::Monitor<Data>;

}  // namespace hwinfo::monitoring::ram
