// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include <hwinfo/mainboard.h>

namespace hwinfo {

// _____________________________________________________________________________________________________________________
const std::string& MainBoard::vendor() const { return _vendor; }

// _____________________________________________________________________________________________________________________
const std::string& MainBoard::name() const { return _name; }

// _____________________________________________________________________________________________________________________
const std::string& MainBoard::version() const { return _version; }

// _____________________________________________________________________________________________________________________
const std::string& MainBoard::serialNumber() const { return _serial_number; }

}  // namespace hwinfo
