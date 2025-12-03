#pragma once

#include <optional>
#include <string>

#include "logic/ports/IOtaPort.hpp"

namespace logic::use_cases {

class OtaCoordinator {
public:
    explicit OtaCoordinator(logic::ports::IOtaPort& rOtaPort);

    std::optional<logic::ports::OtaStatus> optCheckForUpdate();
    bool bPerformUpdate(const std::string& sVersion);

private:
    logic::ports::IOtaPort& m_rOtaPort;
};

}  // namespace logic::use_cases
