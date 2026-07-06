// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <hwinfo/platform.h>

#include <string>

namespace hwinfo {

class HWINFO_API MainBoard {
  friend std::string get_dmi_by_name(const std::string& name);

 public:
  MainBoard();
  ~MainBoard() = default;

  HWI_NODISCARD const std::string& vendor() const;
  HWI_NODISCARD const std::string& name() const;
  HWI_NODISCARD const std::string& version() const;
  HWI_NODISCARD const std::string& serialNumber() const;

 private:
  std::string _vendor;
  std::string _name;
  std::string _version;
  std::string _serial_number;
};

}  // namespace hwinfo
