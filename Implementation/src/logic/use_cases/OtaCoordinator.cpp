#include "logic/use_cases/OtaCoordinator.hpp"

namespace logic::use_cases {

OtaCoordinator::OtaCoordinator(logic::ports::IOtaPort& rOtaPort) : m_rOtaPort{rOtaPort} {}

std::optional<logic::ports::OtaStatus> OtaCoordinator::optCheckForUpdate() {
    return m_rOtaPort.checkForUpdate();
}

bool OtaCoordinator::bPerformUpdate(const std::string& sVersion) {
    return m_rOtaPort.downloadAndInstall(sVersion);
}

}  // namespace logic::use_cases
