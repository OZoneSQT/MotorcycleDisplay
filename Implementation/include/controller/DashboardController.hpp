#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "logic/entities/Manual.hpp"
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
                        const std::optional<logic::entities::ManualPanel>& optManualPanel) = 0;
};

class DashboardController {
public:
    DashboardController(logic::use_cases::CanDataProcessor& rDataProcessor,
                        logic::use_cases::AlertEvaluator& rAlertEvaluator,
                        logic::use_cases::DataLogger& rDataLogger,
                        logic::use_cases::UserManualManager& rManualManager,
                        IDashboardView& rView);

    void processFrame();
    void goHomeManual();
    void goBackManual();
    void openManualTopic(const std::string& sTopicId);
    void refreshManualView();

private:
    void ensureManualLoaded();
    const logic::entities::ManualNode* pCurrentManualNode() const;
    std::optional<logic::entities::ManualPanel> optBuildManualPanel() const;

    logic::use_cases::CanDataProcessor& m_rDataProcessor;
    logic::use_cases::AlertEvaluator& m_rAlertEvaluator;
    logic::use_cases::DataLogger& m_rDataLogger;
    logic::use_cases::UserManualManager& m_rManualManager;
    IDashboardView& m_rView;
    std::optional<logic::entities::ManualNode> m_optManualRoot{};
    std::vector<const logic::entities::ManualNode*> m_vManualStack;
    std::optional<logic::entities::VehicleData> m_optLastData{};
};

}  // namespace controller
