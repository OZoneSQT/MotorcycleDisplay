#pragma once

#include <vector>

#include "logic/entities/Alert.hpp"
#include "logic/use_cases/AlertEvaluator.hpp"

namespace controller {

class AlertController {
public:
    explicit AlertController(logic::use_cases::AlertEvaluator& rEvaluator);

    void updateConfigs(std::vector<logic::entities::AlertConfig> vConfigs);
    [[nodiscard]] const std::vector<logic::entities::AlertState>& vCurrentAlerts() const noexcept;

private:
    logic::use_cases::AlertEvaluator& m_rEvaluator;
};

}  // namespace controller
