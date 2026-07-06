#include "hwinfo/platform.h"

#ifdef HWINFO_APPLE

#include <chrono>
#include <vector>

#include "hwinfo/monitoring/cpu.h"

namespace hwinfo::monitoring::cpu {

std::vector<double> thread_utilization([[maybe_unused]] std::chrono::milliseconds sleep) { return {}; }
double utilization([[maybe_unused]] std::chrono::milliseconds sleep) { return 0.0; }
std::vector<int64_t> thread_frequency_hz() { return {}; }
Data fetch([[maybe_unused]] std::chrono::milliseconds sleep) { return {}; }

}  // namespace hwinfo::monitoring::cpu

#endif  // HWINFO_APPLE
