// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#include <hwinfo/platform.h>

#ifdef HWINFO_WINDOWS

#include <dxgi1_6.h>
#include <windows.h>

#include <algorithm>
#include <string>
#include <vector>

#include "hwinfo/gpu.h"
#include "hwinfo/utils/stringutils.h"
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "setupapi.lib")

#ifdef USE_OCL
#include <hwinfo/opencl/device.h>
#endif

namespace hwinfo {

// _____________________________________________________________________________________________________________________
std::vector<GPU> getAllGPUs() {
  std::vector<GPU> gpus;

  IDXGIFactory1* pFactory;
  if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&pFactory))) return {};

  IDXGIAdapter1* pAdapter;
  for (UINT i = 0; pFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
    DXGI_ADAPTER_DESC1 desc;
    pAdapter->GetDesc1(&desc);

    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
      pAdapter->Release();
      continue;
    }

    GPU gpu;
    gpu._id = static_cast<int>(i);

    std::wstring ws(desc.Description);
    gpu._name = utils::wstring_to_std_string(ws);

    gpu._dedicated_memory_Bytes = static_cast<int64_t>(desc.DedicatedVideoMemory);
    gpu._shared_memory_Bytes = static_cast<int64_t>(desc.SharedSystemMemory);

    char buffer[10];
    sprintf_s(buffer, "0x%04X", desc.VendorId);
    gpu._vendor_id = buffer;
    sprintf_s(buffer, "0x%04X", desc.DeviceId);
    gpu._device_id = buffer;

    if (desc.VendorId == 0x10DE) {
      gpu._vendor = "NVIDIA";
    } else if (desc.VendorId == 0x1002 || desc.VendorId == 0x1022) {
      gpu._vendor = "AMD";
    } else if (desc.VendorId == 0x8086) {
      gpu._vendor = "Intel";
    } else {
      gpu._vendor = "Unknown";
    }

    gpus.push_back(gpu);
    pAdapter->Release();
  }
  pFactory->Release();
#ifdef USE_OCL
  auto cl_gpus = opencl_::DeviceManager::get_list<opencl_::Filter::GPU>();
  for (auto& gpu : gpus) {
    for (auto* cl_gpu : cl_gpus) {
      if (cl_gpu->name() == gpu.name()) {
        gpu._driverVersion = cl_gpu->driver_version();
        gpu._frequency_MHz = static_cast<int64_t>(cl_gpu->clock_frequency_MHz());
        gpu._num_cores = static_cast<int>(cl_gpu->cores());
        gpu._dedicated_memory_Bytes = cl_gpu->memory_Bytes();
        break;
      }
    }
  }
#endif  // USE_OCL
  return gpus;
}

}  // namespace hwinfo

#endif  // HWINFO_WINDOWS
