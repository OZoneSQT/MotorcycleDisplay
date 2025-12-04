#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

#include "logic/entities/VehicleData.hpp"

namespace logic::entities {

enum class AlertSeverity { kInfo, kWarning, kCritical };

enum class AlertType { kSpeed, kRpm, kEngineTemp, kThrottle, kCustom };

struct AlertThreshold {
    AlertType eType{AlertType::kCustom};
    float fLimit{0.F};
    bool bTriggerAbove{true};
};

struct AlertMetadata {
    std::string sId{};
    std::string sMessage{};
    AlertSeverity eSeverity{AlertSeverity::kInfo};
};

struct AlertConfig {
    AlertMetadata stMetadata{};
    AlertThreshold stThreshold{};
    std::optional<std::function<bool(const VehicleData&)>> fnPredicate{};
};

struct AlertState {
    AlertMetadata stMetadata{};
    bool bActive{false};
    std::uint64_t u64TriggeredAtMs{0ULL};
};

}  // namespace logic::entities
