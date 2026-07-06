// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <hwinfo/platform.h>

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace hwinfo {

class HWINFO_API GPU {
  friend std::vector<GPU> getAllGPUs();

 public:
  static constexpr std::uint32_t invalid_id = std::numeric_limits<std::uint32_t>::max();

 public:
  ~GPU() = default;

  HWI_NODISCARD const std::string& vendor() const;
  HWI_NODISCARD const std::string& name() const;
  HWI_NODISCARD const std::string& driverVersion() const;
  HWI_NODISCARD uint64_t dedicated_memory_Bytes() const;
  HWI_NODISCARD uint64_t shared_memory_Bytes() const;
  HWI_NODISCARD uint64_t frequency_hz() const;
  HWI_NODISCARD std::uint64_t num_cores() const;
  HWI_NODISCARD std::uint32_t id() const;
  HWI_NODISCARD const std::string& vendor_id() const;
  HWI_NODISCARD const std::string& device_id() const;

 private:
  GPU() = default;

 private:
  std::string _vendor;
  std::string _name;
  std::string _driverVersion;
  uint64_t _dedicated_memory_Bytes = 0;
  uint64_t _shared_memory_Bytes = 0;
  uint64_t _frequency_hz = 0;
  std::uint64_t _num_cores = 0;
  std::uint32_t _id = invalid_id;
  std::string _vendor_id;
  std::string _device_id;
};

std::vector<GPU> getAllGPUs();

}  // namespace hwinfo
