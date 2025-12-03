#include "ui/DashboardView.hpp"

namespace ui {

DashboardView::DashboardView(driver::DisplayDriver& rDisplayDriver, driver::TouchDriver& rTouchDriver)
    : m_rDisplayDriver{rDisplayDriver}, m_rTouchDriver{rTouchDriver} {}

void DashboardView::render(const logic::entities::VehicleData& stData,
                           const std::vector<logic::entities::AlertState>& vAlerts,
                           const std::optional<std::string>& optManualContent) {
    m_rDisplayDriver.drawDashboard(stData, vAlerts, optManualContent);
}

void DashboardView::processInput(const std::function<void(const driver::TouchEvent&)>& fnHandler) {
    const auto optEvent = m_rTouchDriver.optReadEvent();
    if (optEvent.has_value()) {
        fnHandler(optEvent.value());
    }
}

}  // namespace ui
