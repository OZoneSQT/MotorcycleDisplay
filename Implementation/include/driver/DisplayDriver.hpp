#pragma once

#include <optional>
#include <string>
#include <vector>

#include "logic/entities/Alert.hpp"
#include "logic/entities/VehicleData.hpp"

namespace driver {

class DisplayDriver {
public:
    virtual ~DisplayDriver() = default;
    virtual void initialize() = 0;
    virtual void drawDashboard(const logic::entities::VehicleData& stData,
                               const std::vector<logic::entities::AlertState>& vAlerts,
                               const std::optional<std::string>& optManualContent) = 0;
};

}  // namespace driver
