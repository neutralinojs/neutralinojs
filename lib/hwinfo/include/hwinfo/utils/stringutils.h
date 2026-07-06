// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <algorithm>
#include <clocale>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

namespace hwinfo::utils {

/**
 * Replaces an occurence once in the entire string. Stops at first match
 *
 * @param input The input string
 * @param from The token to match
 * @param to The token to use as a replace with matched ones
 */
inline bool replaceOnce(std::string& input, const std::string& from, const std::string& to) {
  size_t start_pos = input.find(from);

  if (start_pos == std::string::npos) return false;

  input.replace(start_pos, from.length(), to);

  return true;
}

/**
 * Replaces all occurences in the entire string.
 *
 * @param input The input string
 * @param from The token to match
 * @param to The token to use as a replace with matched ones
 */
inline void replaceAll(std::string& input, const char from, const char to) {
  std::replace(input.begin(), input.end(), from, to);
}

/**
 * remove all white spaces (' ', '\t', '\n') from start and end of input
 * inplace!
 * @param input
 */
inline void strip(std::string& input) {
  auto start = input.find_first_not_of(" \t\r\n");
  auto end = input.find_last_not_of(" \t\r\n");
  if (start == std::string::npos) {
    input.clear();
  } else {
    input = input.substr(start, end - start + 1);
  }
}

/**
 * Count occurrences of a substring in input
 * @param input
 * @param substring
 * @return
 */
inline unsigned count_substring(const std::string& input, const std::string& substring) {
  unsigned occurrences = 0;
  std::string::size_type shift = 0;
  while ((shift = input.find(substring, shift)) != std::string::npos) {
    occurrences++;
    shift += substring.size();
  }
  return occurrences;
}

/**
 * Split input string at delimiter and return result
 * @param input
 * @param delimiter
 * @return
 */
inline std::vector<std::string> split(const std::string& input, const std::string& delimiter) {
  std::vector<std::string> result;
  size_t shift = 0;
  while (true) {
    size_t match = input.find(delimiter, shift);
    result.emplace_back(input.substr(shift, match - shift));
    if (match == std::string::npos) {
      break;
    }
    shift = match + delimiter.size();
  }
  return result;
}

/**
 * Split input string at delimiter (char) and return result
 * @param input
 * @param delimiter
 * @return
 */
inline std::vector<std::string> split(const std::string& input, const char delimiter) {
  std::vector<std::string> result;
  size_t shift = 0;
  while (true) {
    size_t match = input.find(delimiter, shift);
    if (match == std::string::npos) {
      break;
    }
    result.emplace_back(input.substr(shift, match - shift));
    shift = match + 1;
  }
  return result;
}

/**
 * split input at delimiter and return substring at position index.
 * index can be negative, where -1 is the last occurrence.
 * @param input
 * @param delimiter
 * @param index
 * @return
 */
inline std::string split_get_index(const std::string& input, const std::string& delimiter, int index) {
  unsigned occ = count_substring(input, delimiter) + 1;
  index = index < 0 ? static_cast<int>(occ + index) : index;
  if (static_cast<int>(occ) <= index) {
    return "";
  }

  std::string::size_type start_index = 0;
  while (true) {
    if (index == 0) {
      break;
    }
    start_index = input.find(delimiter, start_index) + delimiter.size();
    index--;
  }
  std::string::size_type end_index = input.find(delimiter, start_index);
  if (end_index == std::string::npos) {
    return {input.begin() + static_cast<int64_t>(start_index), input.end()};
  }
  return {input.begin() + static_cast<int64_t>(start_index), input.begin() + static_cast<int64_t>(end_index)};
}

/**
 * Convert windows wstring to string
 * @return
 */
inline std::string wstring_to_string() { return ""; }

/**
 * Convert wstring to string
 * @return
 */
inline std::string wstring_to_std_string(const std::wstring& ws) {
  std::string str_locale = std::setlocale(LC_ALL, NULL);

#if _MSC_VER
  std::setlocale(LC_ALL, ".65001");
#else
  std::setlocale(LC_ALL, ".UTF-8");
#endif
  const wchar_t* wch_src = ws.c_str();

#ifdef _MSC_VER
  size_t n_dest_size;
  wcstombs_s(&n_dest_size, nullptr, 0, wch_src, 0);
  n_dest_size++;  // Increase by one for null terminator

  char* ch_dest = new char[n_dest_size];
  memset(ch_dest, 0, n_dest_size);

  size_t n_convert_size;
  wcstombs_s(&n_convert_size, ch_dest, n_dest_size, wch_src,
             n_dest_size - 1);  // subtract one to ignore null terminator

  std::string result_text = ch_dest;
  delete[] ch_dest;
#else
  size_t n_dest_size = std::wcstombs(NULL, wch_src, 0) + 1;
  char* ch_dest = new char[n_dest_size];
  std::memset(ch_dest, 0, n_dest_size);
  std::wcstombs(ch_dest, wch_src, n_dest_size);
  std::string result_text = ch_dest;
  delete[] ch_dest;
#endif

  std::setlocale(LC_ALL, str_locale.c_str());
  return result_text;
}

/**
 * Replace the std::string::starts_with function only available in C++20 and above.
 * @param str
 * @param prefix
 * @return
 */
template <typename string_type, typename prefix_type>
inline bool starts_with(const string_type& str, const prefix_type& prefix) {
#ifdef __cpp_lib_starts_ends_with
  return str.starts_with(prefix);
#else
  return str.rfind(prefix, 0) == 0;
#endif
}

inline std::string join(const std::vector<std::string>& values, const std::string& delim) {
  if (values.empty()) {
    return {};
  }
  if (values.size() == 1) {
    return values[0];
  }
  std::stringstream ss;
  for (std::size_t i = 0; i < values.size() - 1; ++i) {
    ss << values[i] << delim;
  }
  ss << values.back();
  return ss.str();
}

}  // namespace hwinfo::utils
