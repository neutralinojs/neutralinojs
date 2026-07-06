// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <hwinfo/platform.h>

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace hwinfo {

class HWINFO_API Battery {
  friend std::vector<Battery> getAllBatteries();
  friend std::ostream& operator<<(std::ostream& os, const Battery& battery);

 public:
  static constexpr std::uint32_t invalid_id = std::numeric_limits<std::uint32_t>::max();

 public:
  enum class State { CHARGING, DISCHARGING, UNKNOWN };

 public:
  explicit Battery(std::uint32_t = 0);
  ~Battery() = default;

  HWI_NODISCARD const std::string& vendor() const;
  HWI_NODISCARD const std::string& model() const;
  HWI_NODISCARD const std::string& serialNumber() const;
  HWI_NODISCARD const std::string& technology() const;
  HWI_NODISCARD uint32_t energyFull() const;

  double capacity() const;

  HWI_NODISCARD std::uint32_t id() const;

  HWI_NODISCARD uint32_t energyNow() const;
  HWI_NODISCARD bool charging() const;
  HWI_NODISCARD bool discharging() const;
  HWI_NODISCARD State state() const;

 private:
  std::uint32_t _id = invalid_id;
  std::string _vendor = "<unknown>";
  std::string _model = "<unknown>";
  std::string _serial_number = "<unknown>";
  std::string _technology = "<unknown>";
  uint32_t _energyFull = 0;
};

std::vector<Battery> getAllBatteries();

std::ostream& operator<<(std::ostream& os, const Battery::State& state);
std::ostream& operator<<(std::ostream& os, const Battery& battery);

}  // namespace hwinfo
