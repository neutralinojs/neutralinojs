/**
 * Copyright 2023, Leon Freist (https://github.com/lfreist)
 * Author: Leon Freist <freist.leon@gmail.com>
 *
 * This file is part of hwinfo.
 */

#pragma once

#include <hwinfo/platform.h>

#ifdef HWINFO_UNIX

#include <map>
#include <string>

namespace hwinfo {

struct PCIDevice {
  PCIDevice(std::string d_id, std::string d_name);

  const std::string device_id{};
  const std::string device_name{};
  std::map<std::string, std::string> subsystems{};
};

struct PCIVendor {
  PCIVendor(std::string v_id, std::string v_name);

  const PCIDevice& operator[](const std::string& device_id) const;

  const std::string vendor_id{};
  const std::string vendor_name{};
  std::map<std::string, PCIDevice> devices{};

 private:
  PCIDevice _invalid_device{"0000", "invalid"};
};

class PCIMapper {
 public:
  explicit PCIMapper();
  ~PCIMapper() = default;

  const PCIVendor& vendor_from_id(const std::string& vendor_id) const;

  const PCIVendor& operator[](const std::string& vendor_id) const;

 private:
  std::map<std::string, PCIVendor> _vendors{};
  PCIVendor _invalid_vendor{"0000", "invalid"};
};

struct PCI {
  static PCIMapper getMapper();
};

}  // namespace hwinfo

#endif  // HWINFO_UNIX