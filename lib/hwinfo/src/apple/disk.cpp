// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#include <hwinfo/platform.h>

#include <cstdint>

#ifdef HWINFO_APPLE

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOBSD.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOMedia.h>
#include <hwinfo/disk.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include <string>
#include <unordered_map>

namespace hwinfo {

/**
  Converts a CFStringRef to a std::string
 */
std::string cf_to_std(CFStringRef cfString) {
  if (cfString == nullptr) {
    return "<unknown>";
  }

  CFIndex length = CFStringGetLength(cfString);
  CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;

  // Initialize std::string with maxSize and fill with null characters
  auto out = std::string(maxSize, '\0');

  // Fill std::string with the actual string
  auto success = CFStringGetCString(cfString, const_cast<char*>(out.data()), maxSize, kCFStringEncodingUTF8);

  if (!success) {
    return "<unknown>";
  }

  // Resize the string to the actual length
  out.resize(strlen(out.c_str()));
  return out;
}

/**
  Converts a CFNumberRef to a number of type ReturnType
 */
template <typename NumberType>
NumberType cf_to_std(CFNumberRef raw, CFNumberType cfNumberEnum) {
  if (raw == nullptr) {
    return NumberType();
  }

  NumberType out;
  CFNumberGetValue(raw, cfNumberEnum, &out);

  return out;
}

int64_t cf_to_std(CFNumberRef raw) { return cf_to_std<int64_t>(raw, kCFNumberSInt64Type); }

template <typename ReturnType, typename CFType>
ReturnType getIORegistryProperty(io_object_t service, CFStringRef key) {
  // Get the property from I/O Registry
  auto raw = static_cast<CFType>(IORegistryEntryCreateCFProperty(service, key, kCFAllocatorDefault, 0));

  // Convert the property to a output type
  ReturnType out = cf_to_std(raw);

  // Release the property
  if (raw) {
    CFRelease(raw);
  };

  return out;
}

/**
 * Extracts the base disk name (e.g. "disk3") from a
 * BSD device name like "disk3s1s1".
 *
 * @param bsdName A string such as "disk3s1s1"
 * @return "disk3" if bsdName starts with "disk", otherwise returns bsdName
 */
std::string parseBaseDiskName(const std::string& bsdName) {
  // Check if it starts with "disk"
  if (bsdName.rfind("disk", 0) == 0) {
    // skip the first 4 characters ("disk")
    size_t pos = 4;
    // consume all digits (e.g. "3") until we hit a non-digit
    while (pos < bsdName.size() && std::isdigit(static_cast<unsigned char>(bsdName[pos]))) {
      pos++;
    }
    // return the substring that includes "disk" and any trailing digits
    return bsdName.substr(0, pos);
  }
  // fallback if it doesn't start with "disk"
  return bsdName;
}

/**
 * Builds a mapping from BSD device names to mount points using getfsstat().
 *
 * For each mounted device (e.g. "/dev/disk3s1s1" -> "/"), we:
 *   1. Strip "/dev/" -> "disk3s1s1".
 *   2. Extract the base disk name (e.g. "disk3").
 *   3. Store "disk3s1s1" -> "/" in mountMap.
 *   4. Also link "disk3" to the same mount point if not already linked.
 *
 * This lets us find a container disk's mount (e.g. "disk3" -> "/").
 */
std::unordered_map<std::string, std::string> getBSDToMountPointMapping() {
  std::unordered_map<std::string, std::string> mountMap;             // partition -> mount point
  std::unordered_map<std::string, std::string> baseDiskToPartition;  // diskX -> first found "diskXsY"

  int mountCount = getfsstat(nullptr, 0, MNT_NOWAIT);
  if (mountCount <= 0) {
    return mountMap;
  }

  std::vector<struct statfs> mountInfo(mountCount);
  if (getfsstat(mountInfo.data(), mountCount * sizeof(struct statfs), MNT_NOWAIT) == -1) {
    return mountMap;
  }

  for (const auto& entry : mountInfo) {
    std::string fullBSDName = entry.f_mntfromname;  // e.g. "/dev/disk3s1s1"
    std::string mountPath = entry.f_mntonname;      // e.g. "/"

    // Remove the "/dev/" prefix if present
    if (fullBSDName.rfind("/dev/", 0) == 0) {
      fullBSDName.erase(0, 5);
    }

    // Extract the base disk (e.g. "disk3") from e.g. "disk3s1s1"
    std::string baseDisk = parseBaseDiskName(fullBSDName);

    // Store partition -> mount
    mountMap[fullBSDName] = mountPath;

    // Link "disk3" to the first partition we see, if not already linked
    if (baseDiskToPartition.find(baseDisk) == baseDiskToPartition.end()) {
      baseDiskToPartition[baseDisk] = fullBSDName;
    }
  }

  // Link each base disk "diskX" to the same mount as its first partition "diskXsY"
  for (const auto& [disk, partition] : baseDiskToPartition) {
    auto it = mountMap.find(partition);
    if (it != mountMap.end()) {
      mountMap[disk] = it->second;
    }
  }

  return mountMap;
}

/**
 * Retrieves the free disk space (in bytes) for a given mount point
 * by calling statfs().
 *
 * @param mountPoint The path at which the filesystem is mounted (e.g. "/")
 * @return The free space in bytes, or (uint64_t)-1 on error
 */
uint64_t getFreeDiskSpace(const std::string& mountPoint) {
  struct statfs fsStats;

  if (statfs(mountPoint.c_str(), &fsStats) == 0) {
    uint64_t freeSpace = static_cast<uint64_t>(fsStats.f_bavail) * fsStats.f_bsize;
    return freeSpace;
  } else {
    return static_cast<uint64_t>(-1);
  }
}

// Retrieves disk information using I/O Kit
std::vector<Disk> getAllDisks() {
  std::vector<Disk> disks;

  // Build a map from BSD devices (diskXsY) and base disks (diskX) to mount points
  auto mountMap = getBSDToMountPointMapping();

  CFMutableDictionaryRef matchingDict = IOServiceMatching(kIOMediaClass);
  CFDictionaryAddValue(matchingDict, CFSTR(kIOMediaWholeKey), kCFBooleanTrue);

  io_iterator_t iter;
  if (IOServiceGetMatchingServices(0, matchingDict, &iter) == KERN_SUCCESS) {
    int i_disk = 0;
    while (true) {
      auto service = IOIteratorNext(iter);
      if (service == 0) {
        break;
      }

      Disk disk;
      disk._id = i_disk;

      // Retrieve the BSD name (e.g. "disk3")
      std::string bsdName = getIORegistryProperty<std::string, CFStringRef>(service, CFSTR(kIOBSDNameKey));

      // Get disk name
      char model[128];
      if (IORegistryEntryGetName(service, model) != KERN_SUCCESS) {
        disk._model = "<unknown>";
      } else {
        disk._model = model;
      }

      // Guess vendor based on model
      if (disk._model.find("APPLE") != std::string::npos || disk._model.find("Apple") != std::string::npos) {
        disk._vendor = "Apple";
      } else {
        disk._vendor = "<unknown>";
      }

      disk._serial_number = getIORegistryProperty<std::string, CFStringRef>(service, CFSTR(kIOMediaUUIDKey));

      disk._size_bytes = getIORegistryProperty<int64_t, CFNumberRef>(service, CFSTR(kIOMediaSizeKey));

      // If there's no BSD name, we can't look it up in the mount map
      if (!bsdName.empty()) {
        // Look up this BSD device in the mountMap
        if (auto it = mountMap.find(bsdName); it != mountMap.end()) {
          // Get free space for the found mount point
          const std::string& mountPoint = it->second;
          disk._size_bytes = getFreeDiskSpace(mountPoint);

          disk._mount_points.push_back(mountPoint);
        }
      }

      disks.push_back(std::move(disk));

      IOObjectRelease(service);

      i_disk++;
    }
    IOObjectRelease(iter);
  }
  return disks;
}

}  // namespace hwinfo

#endif  // HWINFO_APPLE
