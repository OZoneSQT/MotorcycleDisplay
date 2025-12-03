#pragma once

#include <optional>
#include <string>

#include "logic/ports/IOtaPort.hpp"

namespace driver {

class OtaDriver : public logic::ports::IOtaPort {
public:
    explicit OtaDriver(std::string sUpdateServerUrl);

    std::optional<logic::ports::OtaStatus> checkForUpdate() override;
    bool downloadAndInstall(const std::string& sVersion) override;

private:
    std::string m_sServerUrl;
    std::optional<logic::ports::OtaStatus> m_optCachedStatus{};
};

}  // namespace driver
