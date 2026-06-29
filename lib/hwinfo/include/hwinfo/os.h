// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <hwinfo/platform.h>

#include <string>

namespace hwinfo {

class HWINFO_API OS {
 public:
  OS();
  ~OS() = default;

  HWI_NODISCARD std::string name() const;
  HWI_NODISCARD std::string version() const;
  HWI_NODISCARD std::string kernel() const;
  HWI_NODISCARD bool is32bit() const;
  HWI_NODISCARD bool is64bit() const;
  HWI_NODISCARD bool isBigEndian() const;
  HWI_NODISCARD bool isLittleEndian() const;

 private:
  std::string _name;
  std::string _version;
  std::string _kernel;
  bool _32bit = false;
  bool _64bit = false;
  bool _bigEndian = false;
  bool _littleEndian = false;
};

}  // namespace hwinfo
