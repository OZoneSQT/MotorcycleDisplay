#pragma once

#include <optional>
#include <string>

namespace logic::ports {

struct OtaStatus {
    bool bUpdateAvailable{false};
    std::string sVersion{};
    std::string sReleaseNotes{};
};

class IOtaPort {
public:
    virtual ~IOtaPort() = default;
    virtual std::optional<OtaStatus> checkForUpdate() = 0;
    virtual bool downloadAndInstall(const std::string& sVersion) = 0;
};

}  // namespace logic::ports
