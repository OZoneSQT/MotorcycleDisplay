#include "driver/SystemClock.hpp"

#include <chrono>

namespace driver {

std::uint64_t SystemClock::nowMs() const {
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

}  // namespace driver
