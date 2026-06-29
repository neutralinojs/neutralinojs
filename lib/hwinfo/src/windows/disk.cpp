// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#include "hwinfo/platform.h"

#ifdef HWINFO_WINDOWS

#include <hwinfo/disk.h>
#include <hwinfo/utils/stringutils.h>
#include <windows.h>
#include <winioctl.h>

#include <map>
#include <string>
#include <vector>

namespace {

hwinfo::Disk::Interface mapBusType(STORAGE_BUS_TYPE busType) {
  switch (busType) {
    case BusTypeNvme:
      return hwinfo::Disk::Interface::NVME;
    case BusTypeUsb:
      return hwinfo::Disk::Interface::USB;
    case BusTypeSata:
      return hwinfo::Disk::Interface::SATA;
    case BusTypeScsi:  // fall through
    case BusTypeSas:
      return hwinfo::Disk::Interface::SCSI;
    default:
      return hwinfo::Disk::Interface::UNKNOWN;
  }
}

std::map<uint32_t, std::vector<std::string>> getDiskToVolumeMap() {
  std::map<uint32_t, std::vector<std::string>> mapping;
  char logicalDrives[MAX_PATH] = {};
  GetLogicalDriveStringsA(sizeof(logicalDrives), logicalDrives);

  char* currentDrive = logicalDrives;
  while (*currentDrive) {
    std::string rootPath = currentDrive;  // e.g., "C:\"
    UINT driveType = GetDriveTypeA(rootPath.c_str());

    if (driveType == DRIVE_FIXED || driveType == DRIVE_REMOVABLE) {
      std::string volumePath = "\\\\.\\" + rootPath.substr(0, 2);
      HANDLE hVolume =
          CreateFileA(volumePath.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

      if (hVolume != INVALID_HANDLE_VALUE) {
        STORAGE_DEVICE_NUMBER sdn;
        DWORD bytesReturned;
        if (DeviceIoControl(hVolume, IOCTL_STORAGE_GET_DEVICE_NUMBER, nullptr, 0, &sdn, sizeof(sdn), &bytesReturned,
                            nullptr)) {
          mapping[sdn.DeviceNumber].push_back(rootPath);
        }
        CloseHandle(hVolume);
      }
    }
    currentDrive += strlen(currentDrive) + 1;
  }
  return mapping;
}

}  // namespace

namespace hwinfo {

// _____________________________________________________________________________________________________________________
std::vector<Disk> getAllDisks() {
  std::vector<Disk> disks;

  auto diskToVolumes = getDiskToVolumeMap();

  for (uint32_t i = 0; i < 16; ++i) {
    std::string devicePath = "\\\\.\\PhysicalDrive" + std::to_string(i);

    HANDLE hDevice =
        CreateFileA(devicePath.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

    if (hDevice == INVALID_HANDLE_VALUE) continue;

    Disk disk;
    disk._id = i;

    STORAGE_PROPERTY_QUERY query = {StorageDeviceProperty, PropertyStandardQuery};
    char buffer[1024];
    DWORD bytesReturned;
    if (DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), &buffer, sizeof(buffer),
                        &bytesReturned, nullptr)) {
      auto* desc = reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR*>(buffer);

      disk._interface = mapBusType(desc->BusType);
      if (desc->ProductIdOffset) {
        disk._model = buffer + desc->ProductIdOffset;
      }
      if (desc->VendorIdOffset) {
        disk._vendor = buffer + desc->VendorIdOffset;
      } else {
        static const std::vector<std::string> known_vendors = {"KIOXIA",  "SAMSUNG", "WESTERN DIGITAL", "WD",
                                                               "SEAGATE", "INTEL",   "CRUCIAL",         "KINGSTON"};
        std::string upper_model(disk._model);
        std::transform(disk._model.begin(), disk._model.end(), upper_model.begin(),
                       [](const char c) { return static_cast<char>(std::toupper(static_cast<int>(c))); });
        for (const auto& vendor : known_vendors) {
          if (upper_model.find(vendor) != std::string::npos) {
            disk._vendor = vendor;
            break;
          }
        }
      }
      if (desc->SerialNumberOffset) {
        disk._serial_number = buffer + desc->SerialNumberOffset;
      }
    }

    DISK_GEOMETRY_EX geometry;
    if (DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, nullptr, 0, &geometry, sizeof(geometry),
                        &bytesReturned, nullptr)) {
      disk._size_bytes = geometry.DiskSize.QuadPart;
    }

    if (diskToVolumes.count(i)) {
      disk._mount_points = diskToVolumes[i];
    }

    CloseHandle(hDevice);
    disks.push_back(std::move(disk));
  }
  return disks;
}

}  // namespace hwinfo

#endif  // HWINFO_WINDOWS
