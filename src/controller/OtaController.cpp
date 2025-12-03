#include "controller/OtaController.hpp"

namespace controller {

OtaController::OtaController(logic::use_cases::OtaCoordinator& rCoordinator) : m_rCoordinator{rCoordinator} {}

std::optional<logic::ports::OtaStatus> OtaController::optCheck() {
    return m_rCoordinator.optCheckForUpdate();
}

bool OtaController::bUpdateTo(const std::string& sVersion) {
    return m_rCoordinator.bPerformUpdate(sVersion);
}

}  // namespace controller
