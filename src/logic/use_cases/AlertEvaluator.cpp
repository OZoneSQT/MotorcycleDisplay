#include "logic/use_cases/AlertEvaluator.hpp"

#include <algorithm>

namespace logic::use_cases {

AlertEvaluator::AlertEvaluator(const logic::ports::IClockPort& rClock, logic::ports::IAlertPort& rAlertPort)
    : m_rClock{rClock}, m_rAlertPort{rAlertPort} {}

void AlertEvaluator::configure(std::vector<logic::entities::AlertConfig> vConfigs) {
    m_vConfigs = std::move(vConfigs);
    m_vStates.clear();
    m_vStates.reserve(m_vConfigs.size());
    const auto u64Now = m_rClock.nowMs();
    for (const auto& stConfig : m_vConfigs) {
        m_vStates.push_back(stBuildState(stConfig, u64Now));
    }
}

void AlertEvaluator::evaluate(const logic::entities::VehicleData& stData) {
    const auto u64Now = m_rClock.nowMs();
    for (std::size_t uIndex = 0; uIndex < m_vConfigs.size(); ++uIndex) {
        const auto bTriggered = bIsTriggered(m_vConfigs[uIndex], stData);
        auto& stState = m_vStates[uIndex];
        if (bTriggered && !stState.bActive) {
            stState.bActive = true;
            stState.u64TriggeredAtMs = u64Now;
            m_rAlertPort.onAlertChanged(stState);
        } else if (!bTriggered && stState.bActive) {
            stState.bActive = false;
            stState.u64TriggeredAtMs = 0ULL;
            m_rAlertPort.onAlertChanged(stState);
        }
    }
}

const std::vector<logic::entities::AlertState>& AlertEvaluator::vActiveAlerts() const noexcept {
    return m_vStates;
}

logic::entities::AlertState AlertEvaluator::stBuildState(const logic::entities::AlertConfig& stConfig,
                                                         std::uint64_t u64Timestamp) const {
    logic::entities::AlertState stState{};
    stState.stMetadata = stConfig.stMetadata;
    stState.bActive = false;
    stState.u64TriggeredAtMs = u64Timestamp;
    return stState;
}

bool AlertEvaluator::bIsTriggered(const logic::entities::AlertConfig& stConfig,
                                  const logic::entities::VehicleData& stData) const {
    if (stConfig.fnPredicate.has_value()) {
        return stConfig.fnPredicate.value()(stData);
    }

    const auto fLimit = stConfig.stThreshold.fLimit;
    switch (stConfig.stThreshold.eType) {
        case logic::entities::AlertType::kSpeed:
            return stConfig.stThreshold.bTriggerAbove ? stData.fSpeedKph >= fLimit : stData.fSpeedKph <= fLimit;
        case logic::entities::AlertType::kRpm:
            return stConfig.stThreshold.bTriggerAbove ? stData.fEngineRpm >= fLimit : stData.fEngineRpm <= fLimit;
        case logic::entities::AlertType::kEngineTemp:
            return stConfig.stThreshold.bTriggerAbove ? stData.fEngineTempC >= fLimit : stData.fEngineTempC <= fLimit;
        case logic::entities::AlertType::kThrottle:
            return stConfig.stThreshold.bTriggerAbove ? stData.fThrottlePercent >= fLimit : stData.fThrottlePercent <= fLimit;
        case logic::entities::AlertType::kCustom:
        default:
            return false;
    }
}

}  // namespace logic::use_cases
