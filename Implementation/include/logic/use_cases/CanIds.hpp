#pragma once

#include <cstdint>

namespace logic::use_cases {

struct CanIdConfig {
    std::uint32_t u32Speed;
    std::uint32_t u32Rpm;
    std::uint32_t u32Throttle;
    std::uint32_t u32Abs;
    std::uint32_t u32EngineTemp;
    std::uint32_t u32Battery;
};

inline constexpr CanIdConfig kDefaultCanIds{
    0x100U,  // speed
    0x101U,  // rpm
    0x102U,  // throttle
    0x103U,  // abs
    0x104U,  // engine temp
    0x105U   // battery
};

}  // namespace logic::use_cases
