//
// Created by leon- on 23/06/2023.
//

#pragma once

#include <string>
#include <vector>

namespace hwinfo::utils {

template <typename T>
T get_value(const std::vector<T>& data, size_t index);

template <>
inline std::string get_value<std::string>(const std::vector<std::string>& data, size_t index) {
  if (data.size() < index) {
    return "<unknown>";
  } else {
    return data[index];
  }
}

template <>
inline int64_t get_value<int64_t>(const std::vector<int64_t>& data, size_t index) {
  if (data.size() < index) {
    return -1;
  } else {
    return data[index];
  }
}

template <typename T>
inline bool is_power_of_two(T x) {
  return x > 0 && (x & (x - 1)) == 0;
}

template <typename T>
inline T round_to_next_power_of_2(T val) {
  if (val == 0) return 0;
  if (is_power_of_two(val)) return val;
  val |= val >> 1;
  val |= val >> 2;
  val |= val >> 4;
  val |= val >> 8;
  val |= val >> 16;
  return val + 1;
}

}  // namespace hwinfo::utils