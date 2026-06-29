// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <hwinfo/platform.h>

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace hwinfo {

class HWINFO_API Disk {
 public:
  enum class Interface {
    NVME,
    USB,
    USB1,
    USB2,
    USB3_5GBit,
    USB3_10GBit,
    USB3_20GBit,
    USB4_20GBit,
    USB4_40GBit,
    USB4_80GBit,
    SATA,
    SCSI,
    UNKNOWN
  };

  friend std::vector<Disk> getAllDisks();
  friend std::ostream& operator<<(std::ostream& os, const Disk::Interface& disk_interface);
  friend std::ostream& operator<<(std::ostream& os, const Disk& disk);

 public:
  static constexpr std::uint32_t invalid_id = std::numeric_limits<std::uint32_t>::max();

 public:
  ~Disk() = default;

  HWI_NODISCARD const std::string& vendor() const;
  HWI_NODISCARD const std::string& model() const;
  HWI_NODISCARD const std::string& serial_number() const;
  HWI_NODISCARD std::uint64_t size() const;
  HWI_NODISCARD const std::vector<std::string>& mount_points() const;
  HWI_NODISCARD std::uint32_t id() const;
  HWI_NODISCARD Interface disk_interface() const;

 private:
  Disk() = default;

  std::string _vendor = "<unknown>";
  std::string _model = "<unknown>";
  std::string _serial_number = "<unknown>";
  std::uint64_t _size_bytes = 0;
  std::vector<std::string> _mount_points;
  std::uint32_t _id = invalid_id;
  Interface _interface = Interface::UNKNOWN;
};

std::vector<Disk> getAllDisks();

std::ostream& operator<<(std::ostream& os, const hwinfo::Disk::Interface& disk_interface);

}  // namespace hwinfo
