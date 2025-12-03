#include "controller/AlertController.hpp"

namespace controller {

AlertController::AlertController(logic::use_cases::AlertEvaluator& rEvaluator) : m_rEvaluator{rEvaluator} {}

void AlertController::updateConfigs(std::vector<logic::entities::AlertConfig> vConfigs) {
    m_rEvaluator.configure(std::move(vConfigs));
}

const std::vector<logic::entities::AlertState>& AlertController::vCurrentAlerts() const noexcept {
    return m_rEvaluator.vActiveAlerts();
}

}  // namespace controller
