#pragma once

#include <chrono>
#include <cstdint>
#include <vector>

#include "hwinfo/monitoring/monitor.h"

using namespace std::chrono_literals;

namespace hwinfo::monitoring::cpu {

struct Data {
  double utilization;                       // average across all threads [0, 1]
  std::vector<double> thread_utilization;   // per-thread utilization [0, 1]
  std::vector<int64_t> thread_frequency_hz; // per-thread current clock rate in Hz
};

// Returns average CPU utilization across all logical threads [0, 1].
// Blocks for `sleep` milliseconds to measure the delta.
double utilization(std::chrono::milliseconds sleep = 200ms);

// Returns per-thread utilization [0, 1].
// Blocks for `sleep` milliseconds to measure the delta.
std::vector<double> thread_utilization(std::chrono::milliseconds sleep = 200ms);

// Returns the current clock rate for each logical thread in Hz.
std::vector<int64_t> thread_frequency_hz();

// Fetches a complete Data snapshot. Blocks for `sleep` milliseconds.
Data fetch(std::chrono::milliseconds sleep = 200ms);

using Monitor = hwinfo::monitoring::Monitor<Data>;

}  // namespace hwinfo::monitoring::cpu
