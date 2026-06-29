// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include <hwinfo/platform.h>

#ifdef HWINFO_WINDOWS

#include <hwinfo/cpu.h>
#include <hwinfo/utils/unit.h>
#include <hwinfo/utils/win_registry.h>
#include <intrin.h>
#include <powrprof.h>
#include <winternl.h>

#include <numeric>
#include <string>
#include <thread>
#include <vector>

#pragma comment(lib, "PowrProf.lib")
#pragma comment(lib, "ntdll.lib")

inline int countSetBits(unsigned __int64 mask) {
#if defined(_M_X64) || defined(__x86_64__)
  return static_cast<int>(__popcnt64(mask));
#else
  return static_cast<int>(__popcnt(static_cast<unsigned int>(mask & 0xFFFFFFFF)) +
                          __popcnt(static_cast<unsigned int>(mask >> 32)));
#endif
}

struct PROCESSOR_POWER_INFORMATION {
  ULONG id = std::numeric_limits<ULONG>::max();
  ULONG maxMhz = 0;
  ULONG currentMhz = 0;
  ULONG mhzLimit = 0;
  ULONG maxIdleState = 0;
  ULONG currentIdleState = 0;
};

std::vector<PROCESSOR_POWER_INFORMATION> getProcPowerInfo() {
  SYSTEM_INFO sys_info;
  GetSystemInfo(&sys_info);
  const unsigned num_logicals = sys_info.dwNumberOfProcessors;

  std::vector<PROCESSOR_POWER_INFORMATION> powerInfo(num_logicals);

  NTSTATUS status = CallNtPowerInformation(ProcessorInformation, nullptr, 0, &powerInfo[0],
                                           sizeof(PROCESSOR_POWER_INFORMATION) * num_logicals);

  if (status == 0) {
    return powerInfo;
  }
  return {};
}

namespace hwinfo {

namespace monitor::cpu {

double utilization(std::chrono::milliseconds sleep) {
  auto info = core_utilization(sleep);
  return std::accumulate(info.begin(), info.end(), 0.0,
                         [](const double& a, const double& b) -> double { return a + b; }) /
         static_cast<double>(info.size());
}

std::vector<double> core_utilization(std::chrono::milliseconds sleep) {
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
  for (unsigned i = 0; i < num_logicals; ++i) {
    uint64_t idleDelta = infoB[i].IdleTime.QuadPart - infoA[i].IdleTime.QuadPart;
    uint64_t kernelDelta = infoB[i].KernelTime.QuadPart - infoA[i].KernelTime.QuadPart;
    uint64_t userDelta = infoB[i].UserTime.QuadPart - infoA[i].UserTime.QuadPart;

    uint64_t totalDelta = kernelDelta + userDelta;

    if (totalDelta == 0) {
      results.push_back(0.0);
    } else {
      double util = (1.0 - static_cast<double>(idleDelta) / static_cast<double>(totalDelta));
      results.push_back(std::max(0.0, std::min(1.0, util)));
    }
  }
  return results;
}

// _____________________________________________________________________________________________________________________
std::vector<int64_t> current_frequency_hz() {
  std::vector<int64_t> result;
  for (const auto& info : getProcPowerInfo()) {
    result.emplace_back(info.currentMhz);
  }

  return result;
}

}  // namespace monitor::cpu

// =====================================================================================================================
// _____________________________________________________________________________________________________________________
std::vector<CPU> getAllCPUs() {
  std::vector<CPU> cpus;
  CPU local_cpu;
  local_cpu._id = 0;

  std::wstring reg_cpu_path = L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0";
  local_cpu._modelName =
      internal::utils::getRegistryValue<std::string>(HKEY_LOCAL_MACHINE, reg_cpu_path, L"ProcessorNameString");
  local_cpu._vendor =
      internal::utils::getRegistryValue<std::string>(HKEY_LOCAL_MACHINE, reg_cpu_path, L"VendorIdentifier");

  DWORD bufferSize = 0;
  GetLogicalProcessorInformationEx(RelationAll, nullptr, &bufferSize);
  std::vector<BYTE> buffer(bufferSize);

  if (!GetLogicalProcessorInformationEx(RelationAll, (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)buffer.data(),
                                        &bufferSize)) {
    return {};
  }

  std::vector<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX> coreEntries;
  std::vector<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX> cacheEntries;

  unsigned char* ptr = buffer.data();
  while (ptr < buffer.data() + bufferSize) {
    auto info = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)ptr;
    if (info->Relationship == RelationProcessorCore) coreEntries.push_back(info);
    if (info->Relationship == RelationCache) cacheEntries.push_back(info);
    ptr += info->Size;
  }

  auto regular_frequency =
      internal::utils::getRegistryValue<int64_t>(HKEY_LOCAL_MACHINE, reg_cpu_path, L"~MHz") * unit::SiPrefix::MEGA;

  for (size_t i = 0; i < coreEntries.size(); ++i) {
    auto cInfo = coreEntries[i];
    CPU::Core core{};
    core.id = i;
    core.regular_frequency_hz = regular_frequency;
    core.max_frequency_hz = regular_frequency;

    // SMT is true if logical threads > 1 for this physical core
    int threadsInThisCore = countSetBits(cInfo->Processor.GroupMask[0].Mask);
    core.smt = (threadsInThisCore > 1);

    local_cpu._numPhysicalCores++;
    local_cpu._numLogicalCores += threadsInThisCore;

    // Initialize cache vector [L1 Data, L1 Instruction, L2, L3]
    core.cache = {0, 0, 0, 0};

    for (auto cache : cacheEntries) {
      if ((cInfo->Processor.GroupMask[0].Mask & cache->Cache.GroupMask.Mask) != 0) {
        uint8_t level = cache->Cache.Level;
        auto type = cache->Cache.Type;

        if (level == 1) {
          if (type == CacheData) {
            core.cache.l1_data = cache->Cache.CacheSize;
          } else if (type == CacheInstruction) {
            core.cache.l1_instruction = cache->Cache.CacheSize;
          } else if (type == CacheUnified) {
            // Some CPUs have unified L1 (rare but possible)
            core.cache.l1_data = core.cache.l1_instruction = cache->Cache.CacheSize;
          }
        } else if (level == 2) {
          core.cache.l2 = cache->Cache.CacheSize;
        } else if (level == 3) {
          core.cache.l3 = cache->Cache.CacheSize;
        }
      }
    }
    local_cpu._cores.push_back(core);
  }

  cpus.emplace_back(local_cpu);
  return cpus;
}

}  // namespace hwinfo

#endif  // HWINFO_WINDOWS
