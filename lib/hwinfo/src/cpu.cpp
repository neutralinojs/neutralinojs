// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include <hwinfo/cpu.h>

#include <string>
#include <vector>

namespace hwinfo {

// _____________________________________________________________________________________________________________________
std::uint32_t CPU::id() const { return _id; }

// _____________________________________________________________________________________________________________________
const std::string& CPU::modelName() const { return _modelName; }

// _____________________________________________________________________________________________________________________
const std::string& CPU::vendor() const { return _vendor; }

// _____________________________________________________________________________________________________________________
std::uint64_t CPU::numPhysicalCores() const { return _numPhysicalCores; }

// _____________________________________________________________________________________________________________________
std::uint64_t CPU::numLogicalCores() const { return _numLogicalCores; }

// _____________________________________________________________________________________________________________________
const std::vector<std::string>& CPU::flags() const { return _flags; }

// _____________________________________________________________________________________________________________________
const std::vector<CPU::Core>& CPU::cores() const { return _cores; }

}  // namespace hwinfo
