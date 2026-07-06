// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <hwinfo/platform.h>
#include <hwinfo/utils/wmi_wrapper.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

using namespace std::chrono_literals;

namespace hwinfo {

namespace monitor::cpu {

double utilization(std::chrono::milliseconds sleep = 200ms);
std::vector<double> core_utilization(std::chrono::milliseconds sleep = 200ms);
std::vector<int64_t> current_frequency_hz();

}  // namespace monitor::cpu

class HWINFO_API CPU {
  friend std::vector<CPU> getAllCPUs();

 public:
  static constexpr std::uint32_t invalid_id = std::numeric_limits<std::uint32_t>::max();

 public:
  struct Cache {
    std::uint64_t l1_data;
    std::uint64_t l1_instruction;
    std::uint64_t l2;
    std::uint64_t l3;
  };
  struct Core {
    std::uint64_t id;
    Cache cache;
    std::uint64_t regular_frequency_hz;
    std::uint64_t max_frequency_hz;
    bool smt;
  };

 public:
  ~CPU() = default;

  HWI_NODISCARD std::uint32_t id() const;
  HWI_NODISCARD const std::string& modelName() const;
  HWI_NODISCARD const std::string& vendor() const;
  HWI_NODISCARD std::uint64_t numPhysicalCores() const;
  HWI_NODISCARD std::uint64_t numLogicalCores() const;
  HWI_NODISCARD const std::vector<std::string>& flags() const;
  HWI_NODISCARD const std::vector<Core>& cores() const;

 private:
  CPU() = default;

  std::uint32_t _id = invalid_id;
  std::string _modelName;
  std::string _vendor;
  std::uint64_t _numPhysicalCores = 0;
  std::uint64_t _numLogicalCores = 0;
  std::vector<std::string> _flags;
  std::vector<Core> _cores;
};

std::vector<CPU> getAllCPUs();

}  // namespace hwinfo
