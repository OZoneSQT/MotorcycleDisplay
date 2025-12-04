#pragma once

#include <vector>

#include "logic/entities/Alert.hpp"
#include "logic/entities/VehicleData.hpp"
#include "logic/ports/IAlertPort.hpp"
#include "logic/ports/IClockPort.hpp"

namespace logic::use_cases {

class AlertEvaluator {
public:
    AlertEvaluator(const logic::ports::IClockPort& rClock, logic::ports::IAlertPort& rAlertPort);

    void configure(std::vector<logic::entities::AlertConfig> vConfigs);
    void evaluate(const logic::entities::VehicleData& stData);
    [[nodiscard]] const std::vector<logic::entities::AlertState>& vActiveAlerts() const noexcept;

private:
    logic::entities::AlertState stBuildState(const logic::entities::AlertConfig& stConfig, std::uint64_t u64Timestamp) const;
    bool bIsTriggered(const logic::entities::AlertConfig& stConfig, const logic::entities::VehicleData& stData) const;

    const logic::ports::IClockPort& m_rClock;
    logic::ports::IAlertPort& m_rAlertPort;
    std::vector<logic::entities::AlertConfig> m_vConfigs{};
    std::vector<logic::entities::AlertState> m_vStates{};
};

}  // namespace logic::use_cases
