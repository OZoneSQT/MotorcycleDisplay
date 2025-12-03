#pragma once

#include <array>
#include <cstdint>
#include <optional>

namespace logic::ports {

struct RawCanFrame {
    std::uint32_t u32Id{0U};
    std::array<std::uint8_t, 8U> au8Data{};
    std::uint8_t u8Dlc{0U};
    std::uint64_t u64TimestampMs{0ULL};
};

class ICanPort {
public:
    virtual ~ICanPort() = default;
    virtual bool initialize() = 0;
    virtual std::optional<RawCanFrame> readFrame() = 0;
};

}  // namespace logic::ports
