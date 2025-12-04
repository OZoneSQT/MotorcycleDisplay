#pragma once

#include <cstdint>

namespace logic::ports {

class IClockPort {
public:
    virtual ~IClockPort() = default;
    virtual std::uint64_t nowMs() const = 0;
};

}  // namespace logic::ports
