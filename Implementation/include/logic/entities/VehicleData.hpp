#pragma once

#include <cstdint>

namespace logic::entities {

struct VehicleData {
    float fSpeedKph{0.F};
    float fEngineRpm{0.F};
    float fThrottlePercent{0.F};
    bool bAbsActive{false};
    float fEngineTempC{0.F};
    float fBatteryVoltage{0.F};
    std::uint64_t u64TimestampMs{0ULL};

    [[nodiscard]] bool bIsValid() const noexcept {
        const bool bSpeedsValid = fSpeedKph >= 0.F && fEngineRpm >= 0.F;
        const bool bThrottleValid = fThrottlePercent >= 0.F && fThrottlePercent <= 100.F;
        const bool bTempValid = fEngineTempC > -60.F && fEngineTempC < 200.F;
        const bool bVoltageValid = fBatteryVoltage >= 0.F && fBatteryVoltage <= 18.F;
        return bSpeedsValid && bThrottleValid && bTempValid && bVoltageValid;
    }
};

}  // namespace logic::entities
