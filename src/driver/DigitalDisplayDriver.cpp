#include "driver/DigitalDisplayDriver.hpp"

#include <algorithm>

namespace driver {

void DigitalDisplayDriver::initialize() {
    std::scoped_lock guard{m_mtxMutex};
    m_bInitialized = true;
    m_bHasRendered = false;
    m_vAlerts.clear();
    m_optManualContent.reset();
}

void DigitalDisplayDriver::drawDashboard(const logic::entities::VehicleData& stData,
                                         const std::vector<logic::entities::AlertState>& vAlerts,
                                         const std::optional<std::string>& optManualContent) {
    std::scoped_lock guard{m_mtxMutex};
    if (!m_bInitialized) {
        return;
    }

    m_stLastData = stData;
    m_vAlerts = vAlerts;
    m_optManualContent = optManualContent;
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

std::optional<std::string> DigitalDisplayDriver::optLastManual() const {
    std::scoped_lock guard{m_mtxMutex};
    return m_optManualContent;
}

}  // namespace driver
