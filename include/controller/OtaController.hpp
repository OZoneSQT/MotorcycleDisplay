#pragma once

#include <optional>

#include "logic/ports/IOtaPort.hpp"
#include "logic/use_cases/OtaCoordinator.hpp"

namespace controller {

class OtaController {
public:
    explicit OtaController(logic::use_cases::OtaCoordinator& rCoordinator);

    std::optional<logic::ports::OtaStatus> optCheck();
    bool bUpdateTo(const std::string& sVersion);

private:
    logic::use_cases::OtaCoordinator& m_rCoordinator;
};

}  // namespace controller
