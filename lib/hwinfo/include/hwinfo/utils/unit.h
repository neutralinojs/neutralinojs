#pragma once

#include <cstdint>
#include <type_traits>

#include "hwinfo/utils/utils.h"

namespace hwinfo::unit {

enum class SiPrefix : std::uint64_t {
  KILO = 1000ull,
  MEGA = 1000'000ull,
  GIGA = 1000'000'000ull,
  TERA = 1000'000'000'000ull
};

template <typename T>
inline auto operator*(T val, SiPrefix prefix) {
  if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>) {
    return static_cast<double>(val) * static_cast<double>(prefix);
  } else if constexpr (std::is_unsigned_v<T>) {
    return static_cast<std::uint64_t>(val) * static_cast<std::uint64_t>(prefix);
  } else {
    return static_cast<std::int64_t>(val) * static_cast<std::int64_t>(prefix);
  }
}

enum class IECPrefix : std::uint64_t {
  KIBI = 1024ull,
  MEBI = 1024ull * 1024ull,
  GIBI = 1024ull * 1024ull * 1024ull,
  TEBI = 1024ull * 1024ull * 1024ull * 1024ull
};

template <typename T>
inline auto operator*(T val, IECPrefix prefix) {
  if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>) {
    return static_cast<double>(val) * static_cast<double>(prefix);
  } else if constexpr (std::is_unsigned_v<T>) {
    return static_cast<std::uint64_t>(val) * static_cast<std::uint64_t>(prefix);
  } else {
    return static_cast<std::int64_t>(val) * static_cast<std::int64_t>(prefix);
  }
}

/**
 * @brief Convert bytes to MiB/MB
 *
 * @param bytes number of bytes
 * @return number of MiB as double
 */
inline double unit_prefix_to(std::uint64_t value, IECPrefix prefix) {
  if (utils::is_power_of_two(value)) {
    auto res = static_cast<double>(value / static_cast<std::uint64_t>(prefix));
    if (res != 0) {
      return res;
    }
  }
  return static_cast<double>(value) / static_cast<double>(prefix);
}

inline double unit_prefix_to(std::uint64_t value, SiPrefix prefix) {
  if (utils::is_power_of_two(value)) {
    return static_cast<double>(value / static_cast<std::uint64_t>(prefix));
  }
  return static_cast<double>(value) / static_cast<double>(prefix);
}

}  // namespace hwinfo::unit
