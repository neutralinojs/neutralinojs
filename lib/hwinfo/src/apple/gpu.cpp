// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#include "hwinfo/platform.h"

#ifdef HWINFO_APPLE

#include <regex>
#include <string>
#include <vector>

#include "hwinfo/gpu.h"

namespace hwinfo {

// _____________________________________________________________________________________________________________________
std::vector<GPU> getAllGPUs() {
  std::vector<GPU> gpus{};
  // TODO: implement
  return gpus;
}

}  // namespace hwinfo

#endif  // HWINFO_APPLE