#pragma once

#include "logic/ports/IClockPort.hpp"

namespace driver {

class SystemClock : public logic::ports::IClockPort {
public:
    SystemClock() = default;
    std::uint64_t nowMs() const override;
};

}  // namespace driver
