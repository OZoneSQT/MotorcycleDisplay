#include "controller/DashboardController.hpp"

#include <algorithm>
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

    m_optLastData = optData;
    m_rDataLogger.bLog(*optData);
    m_rAlertEvaluator.evaluate(*optData);

    ensureManualLoaded();
    m_rView.render(*optData, m_rAlertEvaluator.vActiveAlerts(), optBuildManualPanel());
}

void DashboardController::goHomeManual() {
    ensureManualLoaded();
    if (m_vManualStack.size() <= 1U) {
        return;
    }
    const auto* pRoot = m_vManualStack.front();
    m_vManualStack.clear();
    if (pRoot != nullptr) {
        m_vManualStack.push_back(pRoot);
    }
}

void DashboardController::goBackManual() {
    ensureManualLoaded();
    if (m_vManualStack.size() > 1U) {
        m_vManualStack.pop_back();
    }
}

void DashboardController::openManualTopic(const std::string& sTopicId) {
    ensureManualLoaded();
    const auto* pCurrent = pCurrentManualNode();
    if (pCurrent == nullptr) {
        return;
    }
    const auto itChild = std::find_if(pCurrent->vChildren.begin(), pCurrent->vChildren.end(), [&](const auto& stChild) {
        return stChild.sId == sTopicId;
    });
    if (itChild != pCurrent->vChildren.end()) {
        m_vManualStack.push_back(&(*itChild));
    }
}

void DashboardController::refreshManualView() {
    ensureManualLoaded();
    if (!m_optLastData.has_value()) {
        return;
    }
    m_rView.render(*m_optLastData, m_rAlertEvaluator.vActiveAlerts(), optBuildManualPanel());
}

void DashboardController::ensureManualLoaded() {
    if (m_optManualRoot.has_value()) {
        if (m_vManualStack.empty()) {
            m_vManualStack.push_back(&m_optManualRoot.value());
        }
        return;
    }

    const auto optRoot = m_rManualManager.optLoadManualTree();
    if (!optRoot.has_value()) {
        return;
    }

    m_optManualRoot = optRoot;
    m_vManualStack.clear();
    m_vManualStack.push_back(&m_optManualRoot.value());
}

const logic::entities::ManualNode* DashboardController::pCurrentManualNode() const {
    if (m_vManualStack.empty()) {
        return nullptr;
    }
    return m_vManualStack.back();
}

std::optional<logic::entities::ManualPanel> DashboardController::optBuildManualPanel() const {
    const auto* pNode = pCurrentManualNode();
    if (pNode == nullptr) {
        return std::nullopt;
    }

    logic::entities::ManualPanel stPanel{};
    stPanel.sTitle = pNode->sTitle;
    stPanel.sBody = pNode->sBody;
    stPanel.bCanGoBack = m_vManualStack.size() > 1U;
    stPanel.bCanGoHome = stPanel.bCanGoBack;
    stPanel.vChildren.reserve(pNode->vChildren.size());
    for (const auto& stChild : pNode->vChildren) {
        stPanel.vChildren.push_back({stChild.sId, stChild.sTitle});
    }
    return stPanel;
}

}  // namespace controller
