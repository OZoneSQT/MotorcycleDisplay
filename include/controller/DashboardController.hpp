#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "logic/use_cases/AlertEvaluator.hpp"
#include "logic/use_cases/CanDataProcessor.hpp"
#include "logic/use_cases/DataLogger.hpp"
#include "logic/use_cases/UserManualManager.hpp"

namespace controller {

class IDashboardView {
public:
    virtual ~IDashboardView() = default;
    virtual void render(const logic::entities::VehicleData& stData,
                        const std::vector<logic::entities::AlertState>& vAlerts,
                        const std::optional<std::string>& optManualContent) = 0;
};

class DashboardController {
public:
    DashboardController(logic::use_cases::CanDataProcessor& rDataProcessor,
                        logic::use_cases::AlertEvaluator& rAlertEvaluator,
                        logic::use_cases::DataLogger& rDataLogger,
                        logic::use_cases::UserManualManager& rManualManager,
                        IDashboardView& rView);

    void processFrame();

private:
    logic::use_cases::CanDataProcessor& m_rDataProcessor;
    logic::use_cases::AlertEvaluator& m_rAlertEvaluator;
    logic::use_cases::DataLogger& m_rDataLogger;
    logic::use_cases::UserManualManager& m_rManualManager;
    IDashboardView& m_rView;
    std::optional<std::string> m_optCachedManual{};
};

}  // namespace controller
