// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include "hwinfo/platform.h"

#ifdef HWINFO_APPLE

#include <mach/mach.h>
#include <mach/mach_time.h>
#include <sys/sysctl.h>

#include <string>
#include <vector>

#include "hwinfo/cpu.h"
#include "hwinfo/utils/sysctl.h"

namespace hwinfo {

// Helper functions to reduce code duplication
namespace {

// Check if the system is running on Apple Silicon
bool isAppleSilicon() {
  auto machine = utils::getSysctlString("hw.machine");
  return machine.find("arm64") != std::string::npos;
}

// Get the number of physical CPU cores
[[maybe_unused]] int getPhysicalCoreCount() { return utils::getSysctlValue<int>("hw.physicalcpu", 0); }

// Calculate CPU frequency for Apple Silicon - simplified version
uint64_t getCpuFrequency(bool isMax = true) {
  // Try to get CPU frequency directly
  return utils::getSysctlValue<uint64_t>(isMax ? "hw.cpufrequency_max" : "hw.cpufrequency", 0);
}

}  // anonymous namespace

// _____________________________________________________________________________________________________________________
[[maybe_unused]] uint64_t getMaxClockSpeed_MHz() { return getCpuFrequency(true); }

// _____________________________________________________________________________________________________________________
[[maybe_unused]] uint64_t getRegularClockSpeed_MHz() { return getCpuFrequency(false); }

// _____________________________________________________________________________________________________________________
[[maybe_unused]] uint64_t getMinClockSpeed_MHz() {
  return utils::getSysctlValue<uint64_t>("hw.cpufrequency_min", 0) / 1000000;
}

// _____________________________________________________________________________________________________________________
[[maybe_unused]] std::vector<int64_t> currentClockSpeed_MHz() {
  std::vector<int64_t> clockSpeeds;

  processor_info_array_t cpuInfo;
  mach_msg_type_number_t numCpuInfo;
  natural_t numCPUs = 0;
  int num_cores = utils::getSysctlValue<int>("hw.logicalcpu", 0);

  kern_return_t err = host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO, &numCPUs, &cpuInfo, &numCpuInfo);

  if (err == KERN_SUCCESS) {
    // Get CPU frequency
    int64_t freq_mhz = utils::getSysctlValue<uint64_t>("hw.cpufrequency", 0) / 1000000;

    // Fill the vector with the frequency for each logical CPU
    clockSpeeds.resize(num_cores, freq_mhz > 0 ? freq_mhz : -1);

    // Free the processor info when done
    vm_deallocate(mach_task_self(), (vm_address_t)cpuInfo, numCpuInfo * sizeof(natural_t));
  } else {
    // If we can't get processor info, still resize the vector to match the number of cores
    clockSpeeds.resize(num_cores, -1);
  }

  return clockSpeeds;
}

// _____________________________________________________________________________________________________________________
std::string getVendor() {
  // Try to get vendor from sysctl
  auto vendor = utils::getSysctlString("machdep.cpu.vendor", "<unknown>");

  // Check if this is Apple Silicon
  if (vendor == "<unknown>" && isAppleSilicon()) {
    return "Apple";
  }

  return vendor;
}

// _____________________________________________________________________________________________________________________
[[maybe_unused]] double currentUtilisation() {
  host_cpu_load_info_data_t cpuinfo;
  mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;

  static uint64_t lastTotalTicks = 0;
  static uint64_t lastIdleTicks = 0;

  if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&cpuinfo, &count) == KERN_SUCCESS) {
    uint64_t totalTicks = 0;
    for (int i_tick = 0; i_tick < CPU_STATE_MAX; i_tick++) {
      totalTicks += cpuinfo.cpu_ticks[i_tick];
    }

    uint64_t idleTicks = cpuinfo.cpu_ticks[CPU_STATE_IDLE];

    uint64_t totalTicksSinceLastTime = totalTicks - lastTotalTicks;
    uint64_t idleTicksSinceLastTime = idleTicks - lastIdleTicks;

    lastTotalTicks = totalTicks;
    lastIdleTicks = idleTicks;

    if (totalTicksSinceLastTime > 0) {
      return 1.0 - ((double)idleTicksSinceLastTime / totalTicksSinceLastTime);
    }
  }

  return -1.0;
}

// _____________________________________________________________________________________________________________________
[[maybe_unused]] double threadUtilisation(std::uint32_t thread_index) {
  // On macOS, getting per-thread utilization requires more complex code
  // This is a simplified implementation that returns the same value for all threads
  if (thread_index >= 0) {
    processor_cpu_load_info_t cpuLoad;
    mach_msg_type_number_t processorMsgCount;
    natural_t processorCount;

    kern_return_t err = host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO, &processorCount,
                                            (processor_info_array_t*)&cpuLoad, &processorMsgCount);

    if (err == KERN_SUCCESS && thread_index < processorCount) {
      static std::vector<uint64_t> lastTotalTicks;
      static std::vector<uint64_t> lastIdleTicks;

      // Initialize on first call
      if (lastTotalTicks.size() != processorCount) {
        lastTotalTicks.resize(processorCount, 0);
        lastIdleTicks.resize(processorCount, 0);
      }

      uint64_t totalTicks = 0;
      for (int state = 0; state < CPU_STATE_MAX; state++) {
        totalTicks += cpuLoad[thread_index].cpu_ticks[state];
      }

      uint64_t idleTicks = cpuLoad[thread_index].cpu_ticks[CPU_STATE_IDLE];

      uint64_t totalTicksSinceLastTime = totalTicks - lastTotalTicks[thread_index];
      uint64_t idleTicksSinceLastTime = idleTicks - lastIdleTicks[thread_index];

      lastTotalTicks[thread_index] = totalTicks;
      lastIdleTicks[thread_index] = idleTicks;

      vm_deallocate(mach_task_self(), (vm_address_t)cpuLoad,
                    processorMsgCount * sizeof(processor_cpu_load_info_data_t));

      if (totalTicksSinceLastTime > 0) {
        return 1.0 - ((double)idleTicksSinceLastTime / totalTicksSinceLastTime);
      }
    }

    if (err == KERN_SUCCESS) {
      vm_deallocate(mach_task_self(), (vm_address_t)cpuLoad,
                    processorMsgCount * sizeof(processor_cpu_load_info_data_t));
    }
  }

  return -1.0;
}

// _____________________________________________________________________________________________________________________
[[maybe_unused]] std::vector<double> threadsUtilisation() {
  std::vector<double> thread_utility;
  processor_cpu_load_info_t cpuLoad;
  mach_msg_type_number_t processorMsgCount;
  natural_t processorCount;

  kern_return_t err = host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO, &processorCount,
                                          (processor_info_array_t*)&cpuLoad, &processorMsgCount);

  if (err == KERN_SUCCESS) {
    static std::vector<uint64_t> lastTotalTicks;
    static std::vector<uint64_t> lastIdleTicks;

    // Initialize on first call
    if (lastTotalTicks.size() != processorCount) {
      lastTotalTicks.resize(processorCount, 0);
      lastIdleTicks.resize(processorCount, 0);
    }

    thread_utility.resize(processorCount, -1.0);

    for (natural_t i = 0; i < processorCount; i++) {
      uint64_t totalTicks = 0;
      for (int state = 0; state < CPU_STATE_MAX; state++) {
        totalTicks += cpuLoad[i].cpu_ticks[state];
      }

      uint64_t idleTicks = cpuLoad[i].cpu_ticks[CPU_STATE_IDLE];

      uint64_t totalTicksSinceLastTime = totalTicks - lastTotalTicks[i];
      uint64_t idleTicksSinceLastTime = idleTicks - lastIdleTicks[i];

      lastTotalTicks[i] = totalTicks;
      lastIdleTicks[i] = idleTicks;

      if (totalTicksSinceLastTime > 0) {
        thread_utility[i] = 1.0 - ((double)idleTicksSinceLastTime / totalTicksSinceLastTime);
      }
    }

    vm_deallocate(mach_task_self(), (vm_address_t)cpuLoad, processorMsgCount * sizeof(processor_cpu_load_info_data_t));
  }

  return thread_utility;
}

// _____________________________________________________________________________________________________________________
std::string getModelName() {
  std::string name = utils::getSysctlString("machdep.cpu.brand_string", "<unknown> ");
  name.pop_back();
  return name;
}

int getNumLogicalCores();

// _____________________________________________________________________________________________________________________
int getNumPhysicalCores() { return utils::getSysctlValue<int>("hw.physicalcpu", 0); }

// _____________________________________________________________________________________________________________________
int getNumLogicalCores() { return utils::getSysctlValue<int>("hw.logicalcpu", 0); }

[[maybe_unused]] int64_t getL1CacheSize_Bytes() { return utils::getSysctlValue<int64_t>("hw.l1dcachesize", -1); }

[[maybe_unused]] int64_t getL2CacheSize_Bytes() { return utils::getSysctlValue<int64_t>("hw.l2cachesize", -1); }

[[maybe_unused]] int64_t getL3CacheSize_Bytes() { return utils::getSysctlValue<int64_t>("hw.l3cachesize", -1); }

// _____________________________________________________________________________________________________________________
std::vector<CPU> getAllCPUs() {
  std::vector<CPU> cpus;
  CPU cpu;

  cpu._vendor = getVendor();
  cpu._modelName = getModelName();
  cpu._numPhysicalCores = getNumPhysicalCores();
  cpu._numLogicalCores = getNumLogicalCores();

  cpus.push_back(cpu);

  return cpus;
}

}  // namespace hwinfo

#endif  // HWINFO_APPLE
