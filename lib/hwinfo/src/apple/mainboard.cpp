// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include "hwinfo/platform.h"

#ifdef HWINFO_APPLE

#include <Availability.h>
#include <IOKit/IOKitLib.h>

#include "hwinfo/mainboard.h"

#if defined(__MAC_12_0) && __MAC_OS_X_VERSION_MAX_ALLOWED >= __MAC_12_0
#define SAFE_IO_MAIN_PORT kIOMainPortDefault
#else
#define SAFE_IO_MAIN_PORT kIOMasterPortDefault
#endif

namespace hwinfo {

std::string get_mainboard_property(CFStringRef property_name) {
  auto platformExpert = IOServiceGetMatchingService(SAFE_IO_MAIN_PORT, IOServiceMatching("IOPlatformExpertDevice"));
  if (!platformExpert) {
    return "<unknown>";
  }

  auto propertyRef = IORegistryEntryCreateCFProperty(platformExpert, property_name, kCFAllocatorDefault, 0);

  IOObjectRelease(platformExpert);

  if (!propertyRef) {
    return "<unknown>";
  }

  std::string result;

  if (CFTypeID typeID = CFGetTypeID(propertyRef); typeID == CFStringGetTypeID()) {
    auto stringRef = static_cast<CFStringRef>(propertyRef);
    auto length = CFStringGetLength(stringRef);
    auto maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;

    auto buffer = std::string(maxSize, '\0');
    if (CFStringGetCString(stringRef, buffer.data(), maxSize, kCFStringEncodingUTF8)) {
      result = buffer.c_str();  // Remove trailing nulls
    }
  } else if (typeID == CFDataGetTypeID()) {
    const auto dataRef = static_cast<CFDataRef>(propertyRef);
    const auto length = CFDataGetLength(dataRef);
    const auto bytes = CFDataGetBytePtr(dataRef);
    result = std::string(reinterpret_cast<const char*>(bytes), length);

    if (!result.empty() && result.back() == '\0') {
      result.pop_back();
    }
  }

  CFRelease(propertyRef);
  return result.empty() ? "<unknown>" : result;
}

// _____________________________________________________________________________________________________________________
MainBoard::MainBoard() {
  _vendor = get_mainboard_property(CFSTR("manufacturer"));
  _name = "<unknown>";
  _version = "<unknown>";
  _serial_number = get_mainboard_property(CFSTR(kIOPlatformSerialNumberKey));
}

}  // namespace hwinfo

#endif  // HWINFO_APPLE