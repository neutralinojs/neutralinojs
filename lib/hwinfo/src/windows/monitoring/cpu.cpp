#include "hwinfo/platform.h"

#ifdef HWINFO_WINDOWS

#include <windows.h>
#include <powrprof.h>
#include <winternl.h>

#include <numeric>
#include <thread>
#include <vector>

#include "hwinfo/monitoring/cpu.h"

#pragma comment(lib, "PowrProf.lib")
#pragma comment(lib, "ntdll.lib")

namespace {

struct PROCESSOR_POWER_INFORMATION {
  ULONG id;
  ULONG maxMhz;
  ULONG currentMhz;
  ULONG mhzLimit;
  ULONG maxIdleState;
  ULONG currentIdleState;
};

}  // namespace

namespace hwinfo::monitoring::cpu {

std::vector<double> thread_utilization(std::chrono::milliseconds sleep) {
  SYSTEM_INFO sys_info;
  GetSystemInfo(&sys_info);
  const unsigned num_logicals = sys_info.dwNumberOfProcessors;

  std::vector<SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION> infoA(num_logicals);
  std::vector<SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION> infoB(num_logicals);

  auto getPerf = [&](std::vector<SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION>& info) {
    NtQuerySystemInformation((SYSTEM_INFORMATION_CLASS)0x08, info.data(),
                             sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * num_logicals, nullptr);
  };

  getPerf(infoA);
  std::this_thread::sleep_for(sleep);
  getPerf(infoB);

  std::vector<double> results;
  results.reserve(num_logicals);
  for (unsigned i = 0; i < num_logicals; ++i) {
    const uint64_t idle = infoB[i].IdleTime.QuadPart - infoA[i].IdleTime.QuadPart;
    const uint64_t kernel = infoB[i].KernelTime.QuadPart - infoA[i].KernelTime.QuadPart;
    const uint64_t user = infoB[i].UserTime.QuadPart - infoA[i].UserTime.QuadPart;
    const uint64_t total = kernel + user;
    if (total == 0) {
      results.push_back(0.0);
    } else {
      results.push_back(
          std::max(0.0, std::min(1.0, 1.0 - static_cast<double>(idle) / static_cast<double>(total))));
    }
  }
  return results;
}

double utilization(std::chrono::milliseconds sleep) {
  const auto tu = thread_utilization(sleep);
  if (tu.empty()) return 0.0;
  return std::accumulate(tu.begin(), tu.end(), 0.0) / static_cast<double>(tu.size());
}

std::vector<int64_t> thread_frequency_hz() {
  SYSTEM_INFO sys_info;
  GetSystemInfo(&sys_info);
  const unsigned num_logicals = sys_info.dwNumberOfProcessors;

  std::vector<PROCESSOR_POWER_INFORMATION> powerInfo(num_logicals);
  const NTSTATUS status = CallNtPowerInformation(ProcessorInformation, nullptr, 0, powerInfo.data(),
                                                 sizeof(PROCESSOR_POWER_INFORMATION) * num_logicals);
  if (status != 0) return {};

  std::vector<int64_t> result;
  result.reserve(num_logicals);
  for (const auto& info : powerInfo) {
    result.push_back(static_cast<int64_t>(info.currentMhz) * 1'000'000LL);
  }
  return result;
}

Data fetch(std::chrono::milliseconds sleep) {
  auto tu = thread_utilization(sleep);
  auto tf = thread_frequency_hz();
  const double avg =
      tu.empty() ? 0.0 : std::accumulate(tu.begin(), tu.end(), 0.0) / static_cast<double>(tu.size());
  return Data{avg, std::move(tu), std::move(tf)};
}

}  // namespace hwinfo::monitoring::cpu

#endif  // HWINFO_WINDOWS
