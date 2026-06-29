// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include <hwinfo/platform.h>

#ifdef HWINFO_WINDOWS

#include <hwinfo/mainboard.h>
#include <hwinfo/utils/stringutils.h>
#include <hwinfo/utils/wmi_wrapper.h>

#include <string>

namespace hwinfo {

// _____________________________________________________________________________________________________________________
MainBoard::MainBoard() {
  utils::WMI::_WMI wmi;
  const std::wstring query_string(L"SELECT Manufacturer, Product, Version, SerialNumber FROM Win32_BaseBoard");
  bool success = wmi.execute_query(query_string);
  if (!success) {
    return;
  }
  ULONG u_return = 0;
  IWbemClassObject* obj = nullptr;
  wmi.enumerator->Next(WBEM_INFINITE, 1, &obj, &u_return);
  if (!u_return) {
    return;
  }
  VARIANT vt_prop;
  HRESULT hr;
  hr = obj->Get(L"Manufacturer", 0, &vt_prop, nullptr, nullptr);
  if (SUCCEEDED(hr) && (V_VT(&vt_prop) == VT_BSTR)) {
    _vendor = utils::wstring_to_std_string(vt_prop.bstrVal);
  }
  hr = obj->Get(L"Product", 0, &vt_prop, nullptr, nullptr);
  if (SUCCEEDED(hr) && (V_VT(&vt_prop) == VT_BSTR)) {
    _name = utils::wstring_to_std_string(vt_prop.bstrVal);
  }
  hr = obj->Get(L"Version", 0, &vt_prop, nullptr, nullptr);
  if (SUCCEEDED(hr) && (V_VT(&vt_prop) == VT_BSTR)) {
    _version = utils::wstring_to_std_string(vt_prop.bstrVal);
  }
  hr = obj->Get(L"SerialNumber", 0, &vt_prop, nullptr, nullptr);
  if (SUCCEEDED(hr) && (V_VT(&vt_prop) == VT_BSTR)) {
    _serial_number = utils::wstring_to_std_string(vt_prop.bstrVal);
  }
  VariantClear(&vt_prop);
  obj->Release();
}

}  // namespace hwinfo

#endif  // HWINFO_WINDOWS