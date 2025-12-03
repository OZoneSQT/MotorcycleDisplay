#include "controller/DashboardController.hpp"

#include <utility>

namespace controller {

DashboardController::DashboardController(logic::use_cases::CanDataProcessor& rDataProcessor,
                                         logic::use_cases::AlertEvaluator& rAlertEvaluator,
                                         logic::use_cases::DataLogger& rDataLogger,
                                         logic::use_cases::UserManualManager& rManualManager,
                                         IDashboardView& rView)
    : m_rDataProcessor{rDataProcessor},
      m_rAlertEvaluator{rAlertEvaluator},
      m_rDataLogger{rDataLogger},
      m_rManualManager{rManualManager},
      m_rView{rView} {}

void DashboardController::processFrame() {
    const auto optData = m_rDataProcessor.optPollOnce();
    if (!optData.has_value()) {
        return;
    }

    if (!optData->bIsValid()) {
        return;
    }

    m_rDataLogger.bLog(*optData);
    m_rAlertEvaluator.evaluate(*optData);

    if (!m_optCachedManual.has_value()) {
        m_optCachedManual = m_rManualManager.optLoadManual();
    }

    m_rView.render(*optData, m_rAlertEvaluator.vActiveAlerts(), m_optCachedManual);
}

}  // namespace controller
