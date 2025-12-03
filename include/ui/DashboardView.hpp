#pragma once

#include <functional>
#include <optional>
#include <vector>

#include "controller/DashboardController.hpp"
#include "driver/DisplayDriver.hpp"
#include "driver/TouchDriver.hpp"

namespace ui {

class DashboardView : public controller::IDashboardView {
public:
    DashboardView(driver::DisplayDriver& rDisplayDriver, driver::TouchDriver& rTouchDriver);

    void render(const logic::entities::VehicleData& stData,
                const std::vector<logic::entities::AlertState>& vAlerts,
                const std::optional<std::string>& optManualContent) override;

    void processInput(const std::function<void(const driver::TouchEvent&)>& fnHandler);

private:
    driver::DisplayDriver& m_rDisplayDriver;
    driver::TouchDriver& m_rTouchDriver;
};

}  // namespace ui
