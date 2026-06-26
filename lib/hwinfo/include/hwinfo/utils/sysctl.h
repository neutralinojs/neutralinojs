#pragma once

#include <hwinfo/platform.h>

#ifdef HWINFO_APPLE

#include <sys/sysctl.h>

#include <string>

namespace hwinfo::utils {

// Get a string value from sysctl
inline std::string getSysctlString(const char* name, std::string defaultValue = "", size_t initialSize = 256) {
  auto buffer = std::string(initialSize, '\0');
  size_t size = buffer.size();
  if (sysctlbyname(name, buffer.data(), &size, nullptr, 0) == 0) {
    buffer.resize(size);
    return buffer;
  }
  return defaultValue;
}

// Get an integer value from sysctl
template <typename T>
T getSysctlValue(const char* name, T defaultValue = T()) {
  T value;
  size_t size = sizeof(value);
  if (sysctlbyname(name, &value, &size, nullptr, 0) == 0) {
    return value;
  }
  return defaultValue;
}

}  // namespace hwinfo::utils

#endif  // HWINFO_APPLE
