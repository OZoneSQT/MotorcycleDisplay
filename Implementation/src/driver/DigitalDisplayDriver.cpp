#include "driver/DigitalDisplayDriver.hpp"

#include <algorithm>

#include "logic/entities/Manual.hpp"

namespace driver {

void DigitalDisplayDriver::initialize() {
    std::scoped_lock guard{m_mtxMutex};
    m_bInitialized = true;
    m_bHasRendered = false;
    m_vAlerts.clear();
    m_optManualPanel.reset();
}

void DigitalDisplayDriver::drawDashboard(const logic::entities::VehicleData& stData,
                                         const std::vector<logic::entities::AlertState>& vAlerts,
                                         const std::optional<logic::entities::ManualPanel>& optManualPanel) {
    std::scoped_lock guard{m_mtxMutex};
    if (!m_bInitialized) {
        return;
    }

    m_stLastData = stData;
    m_vAlerts = vAlerts;
    m_optManualPanel = optManualPanel;
    m_bHasRendered = true;
}

bool DigitalDisplayDriver::bInitialized() const noexcept {
    std::scoped_lock guard{m_mtxMutex};
    return m_bInitialized;
}

bool DigitalDisplayDriver::bHasRendered() const noexcept {
    std::scoped_lock guard{m_mtxMutex};
    return m_bHasRendered;
}

logic::entities::VehicleData DigitalDisplayDriver::stLastData() const {
    std::scoped_lock guard{m_mtxMutex};
    return m_stLastData;
}

std::vector<logic::entities::AlertState> DigitalDisplayDriver::vLastAlerts() const {
    std::scoped_lock guard{m_mtxMutex};
    return m_vAlerts;
}

std::optional<logic::entities::ManualPanel> DigitalDisplayDriver::optLastManual() const {
    std::scoped_lock guard{m_mtxMutex};
    return m_optManualPanel;
}

}  // namespace driver
