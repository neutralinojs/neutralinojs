#pragma once

#include <hwinfo/platform.h>

#include <string>
#include <vector>

namespace hwinfo {

class HWINFO_API Network {
  friend std::vector<Network> getAllNetworks();

 public:
  ~Network() = default;

  HWI_NODISCARD const std::string& interfaceIndex() const;
  HWI_NODISCARD const std::string& description() const;
  HWI_NODISCARD const std::string& mac() const;
  HWI_NODISCARD const std::string& ip4() const;
  HWI_NODISCARD const std::string& ip6() const;

 private:
  Network() = default;

  std::string _index;
  std::string _description;
  std::string _mac;
  std::string _ip4;
  std::string _ip6;
};

std::vector<Network> getAllNetworks();

}  // namespace hwinfo