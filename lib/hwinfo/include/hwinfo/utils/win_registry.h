#pragma once

#include <hwinfo/platform.h>

#ifdef HWINFO_WINDOWS

#include <windows.h>

#include <string>
#include <type_traits>
#include <vector>

#include "hwinfo/utils/stringutils.h"

namespace hwinfo::internal::utils {

template <typename T>
T getRegistryValue(HKEY hKeyParent, const std::wstring& subkey, const std::wstring& valueName) {
  HKEY hKey;
  if (RegOpenKeyExW(hKeyParent, subkey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
    return T{};
  }

  T result{};
  DWORD dwType = 0;
  DWORD dwSize = 0;

  if (RegQueryValueExW(hKey, valueName.c_str(), nullptr, &dwType, nullptr, &dwSize) == ERROR_SUCCESS) {
    if constexpr (std::is_same_v<T, std::string>) {
      std::wstring wbuffer(dwSize / sizeof(wchar_t), L'\0');
      if (RegQueryValueExW(hKey, valueName.c_str(), nullptr, nullptr, (LPBYTE)wbuffer.data(), &dwSize) ==
          ERROR_SUCCESS) {
        if (!wbuffer.empty() && wbuffer.back() == L'\0') wbuffer.pop_back();
        std::wstring tmp(wbuffer.begin(), wbuffer.end());
        result = hwinfo::utils::wstring_to_std_string(tmp);
      }
    } else if constexpr (std::is_integral_v<T>) {
      DWORD dwResult = 0;
      if (RegQueryValueExW(hKey, valueName.c_str(), nullptr, nullptr, (LPBYTE)&dwResult, &dwSize) == ERROR_SUCCESS) {
        result = static_cast<T>(dwResult);
      }
    }
  }

  RegCloseKey(hKey);
  return result;
}

}  // namespace hwinfo::internal::utils

#endif