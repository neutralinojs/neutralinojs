#include "hwinfo/platform.h"

#ifdef HWINFO_WINDOWS

#include <hwinfo/network.h>
#include <hwinfo/utils/stringutils.h>
#include <hwinfo/utils/wmi_wrapper.h>

namespace hwinfo {

// _____________________________________________________________________________________________________________________
std::vector<Network> getAllNetworks() {
  utils::WMI::_WMI wmi;
  const std::wstring query_string(
      L"SELECT InterfaceIndex, IPAddress, Description, MACAddress "
      L"FROM Win32_NetworkAdapterConfiguration");
  bool success = wmi.execute_query(query_string);
  if (!success) {
    return {};
  }
  std::vector<Network> networks;

  ULONG u_return = 0;
  IWbemClassObject* obj = nullptr;
  while (wmi.enumerator) {
    wmi.enumerator->Next(WBEM_INFINITE, 1, &obj, &u_return);
    if (!u_return) {
      break;
    }
    Network network;
    VARIANT vt_prop;
    HRESULT hr;
    hr = obj->Get(L"InterfaceIndex", 0, &vt_prop, nullptr, nullptr);
    if (SUCCEEDED(hr)) {
      network._index = std::to_string(vt_prop.uintVal);
    }
    hr = obj->Get(L"IPAddress", 0, &vt_prop, nullptr, nullptr);
    if (SUCCEEDED(hr)) {
      if (vt_prop.vt == (VT_ARRAY | VT_BSTR)) {
        LONG lbound, ubound;
        SafeArrayGetLBound(vt_prop.parray, 1, &lbound);
        SafeArrayGetUBound(vt_prop.parray, 1, &ubound);
        std::string ipv4, ipv6;
        for (LONG i = lbound; i <= ubound; ++i) {
          BSTR bstr;
          SafeArrayGetElement(vt_prop.parray, &i, &bstr);
          std::wstring ws(bstr, SysStringLen(bstr));
          std::string ip = utils::wstring_to_std_string(ws);
          if (ip.find(':') != std::string::npos) {
            if (ip.find("fe80::") == 0) {
              ipv6 = ip;
            } else {
              ipv6 = "";
            }
          } else {
            ipv4 = ip;
          }
          SysFreeString(bstr);
        }
        network._ip4 = ipv4;
        network._ip6 = ipv6;
      }
    }
    hr = obj->Get(L"Description", 0, &vt_prop, nullptr, nullptr);
    if (SUCCEEDED(hr)) {
      if (vt_prop.vt == VT_BSTR) {
        network._description = utils::wstring_to_std_string(vt_prop.bstrVal);
      }
    }
    hr = obj->Get(L"MACAddress", 0, &vt_prop, nullptr, nullptr);
    if (SUCCEEDED(hr)) {
      if (vt_prop.vt == VT_BSTR) {
        network._mac = utils::wstring_to_std_string(vt_prop.bstrVal);
      }
    }
    VariantClear(&vt_prop);
    obj->Release();
    networks.push_back(std::move(network));
  }
  return networks;
}

}  // namespace hwinfo

#endif  // HWINFO_WINDOWS