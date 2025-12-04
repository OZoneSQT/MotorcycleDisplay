#pragma once

#include <mutex>

#include "driver/DisplayDriver.hpp"

namespace driver {

class ConsoleDisplayDriver : public DisplayDriver {
public:
    void initialize() override;
    void drawDashboard(const logic::entities::VehicleData& stData,
                       const std::vector<logic::entities::AlertState>& vAlerts,
                       const std::optional<logic::entities::ManualPanel>& optManualPanel) override;

private:
    std::mutex m_mutex;
};

}  // namespace driver
