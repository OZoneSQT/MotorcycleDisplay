#pragma once

#include <mutex>
#include <optional>
#include <vector>

#include "driver/DisplayDriver.hpp"

namespace driver {

class DigitalDisplayDriver : public DisplayDriver {
public:
    void initialize() override;
    void drawDashboard(const logic::entities::VehicleData& stData,
                       const std::vector<logic::entities::AlertState>& vAlerts,
                       const std::optional<std::string>& optManualContent) override;

    [[nodiscard]] bool bInitialized() const noexcept;
    [[nodiscard]] bool bHasRendered() const noexcept;
    [[nodiscard]] logic::entities::VehicleData stLastData() const;
    [[nodiscard]] std::vector<logic::entities::AlertState> vLastAlerts() const;
    [[nodiscard]] std::optional<std::string> optLastManual() const;

private:
    mutable std::mutex m_mtxMutex;
    bool m_bInitialized{false};
    bool m_bHasRendered{false};
    logic::entities::VehicleData m_stLastData{};
    std::vector<logic::entities::AlertState> m_vAlerts{};
    std::optional<std::string> m_optManualContent{};
};

}  // namespace driver
