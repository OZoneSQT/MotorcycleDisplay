#pragma once

#include <optional>
#include <vector>

#include "logic/entities/Alert.hpp"
#include "logic/entities/Manual.hpp"
#include "logic/entities/VehicleData.hpp"

namespace driver {

class DisplayDriver {
public:
    virtual ~DisplayDriver() = default;
    virtual void initialize() = 0;
    virtual void drawDashboard(const logic::entities::VehicleData& stData,
                               const std::vector<logic::entities::AlertState>& vAlerts,
                               const std::optional<logic::entities::ManualPanel>& optManualPanel) = 0;
};

}  // namespace driver
