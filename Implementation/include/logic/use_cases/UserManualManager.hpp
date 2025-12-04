#pragma once

#include <optional>
#include <string>

#include "logic/entities/Manual.hpp"
#include "logic/ports/IStoragePort.hpp"

namespace logic::use_cases {

class UserManualManager {
public:
    explicit UserManualManager(logic::ports::IStoragePort& rStoragePort);

    bool bSetManualPath(std::string sPath);
    std::optional<logic::entities::ManualNode> optLoadManualTree() const;

private:
    logic::ports::IStoragePort& m_rStoragePort;
    std::string m_sManualPath{};
};

}  // namespace logic::use_cases
