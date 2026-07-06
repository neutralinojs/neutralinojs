// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#include "hwinfo/battery.h"

#include <ostream>

namespace hwinfo {

// =====================================================================================================================
// _____________________________________________________________________________________________________________________
Battery::Battery(std::uint32_t id) { _id = id; }

// _____________________________________________________________________________________________________________________
std::uint32_t Battery::id() const { return _id; }

// _____________________________________________________________________________________________________________________
const std::string& Battery::vendor() const { return _vendor; }

// _____________________________________________________________________________________________________________________
const std::string& Battery::model() const { return _model; }

// _____________________________________________________________________________________________________________________
const std::string& Battery::serialNumber() const { return _serial_number; }

// _____________________________________________________________________________________________________________________
const std::string& Battery::technology() const { return _technology; }

// _____________________________________________________________________________________________________________________
uint32_t Battery::energyFull() const { return _energyFull; }

// _____________________________________________________________________________________________________________________
bool Battery::charging() const { return state() == State::CHARGING; }

// _____________________________________________________________________________________________________________________
bool Battery::discharging() const { return state() == State::DISCHARGING; }

// _____________________________________________________________________________________________________________________
double Battery::capacity() const {
  std::uint32_t full = energyFull();
  if (full == 0) {
    return 0.f;
  }
  return static_cast<double>(energyNow()) / static_cast<double>(full);
}

// =====================================================================================================================
// _____________________________________________________________________________________________________________________
std::ostream& operator<<(std::ostream& os, const Battery& battery) {
  os << "Battery{.id=" << battery._id << "', .vendor='" << battery._vendor << "', .model='" << battery._model
     << "', .serial_number='" << battery._serial_number << "', technology='" << battery._technology
     << "', .full_capacity=" << battery._energyFull << ", .state='" << battery.state() << "'}";
  return os;
}

// _____________________________________________________________________________________________________________________
std::ostream& operator<<(std::ostream& os, const Battery::State& state) {
  switch (state) {
    case Battery::State::CHARGING:
      os << "Battery::State::CHARGING";
      break;
    case Battery::State::DISCHARGING:
      os << "Battery::State::DISCHARGING";
      break;
    case Battery::State::UNKNOWN:
      os << "Battery::State::UNKNOWN";
      break;
  }
  return os;
}

}  // namespace hwinfo
