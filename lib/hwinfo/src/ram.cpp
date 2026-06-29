// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include <hwinfo/ram.h>

namespace hwinfo {

// _____________________________________________________________________________________________________________________
const std::vector<Memory::Module>& Memory::modules() const { return _modules; }

}  // namespace hwinfo
