#include <hwinfo/platform.h>

#ifdef HWINFO_APPLE
#include <hwinfo/network.h>

#include <vector>
namespace hwinfo {
std::vector<Network> getAllNetworks() {
  std::vector<Network> networks;
  return networks;
}
}  // namespace hwinfo

#endif  // HWINFO_APPLE
