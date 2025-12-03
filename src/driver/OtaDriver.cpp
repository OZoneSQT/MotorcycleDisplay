#include "driver/OtaDriver.hpp"

#include <fstream>
#include <sstream>

namespace {
struct Manifest {
    std::string version{};
    std::string notes{};
};

std::optional<Manifest> optParseManifest(std::istream& rStream) {
    Manifest stManifest;
    std::string sLine;
    while (std::getline(rStream, sLine)) {
        const auto uSeparator = sLine.find('=');
        if (uSeparator == std::string::npos) {
            continue;
        }
        const auto sKey = sLine.substr(0U, uSeparator);
        const auto sValue = sLine.substr(uSeparator + 1U);
        if (sKey == "version") {
            stManifest.version = sValue;
        } else if (sKey == "notes") {
            stManifest.notes = sValue;
        }
    }
    if (stManifest.version.empty()) {
        return std::nullopt;
    }
    return stManifest;
}

}  // namespace

namespace driver {

OtaDriver::OtaDriver(std::string sUpdateServerUrl) : m_sServerUrl{std::move(sUpdateServerUrl)} {}

std::optional<logic::ports::OtaStatus> OtaDriver::checkForUpdate() {
    std::ifstream file{m_sServerUrl};
    if (!file.is_open()) {
        return std::nullopt;
    }
    const auto optManifest = optParseManifest(file);
    if (!optManifest.has_value()) {
        return std::nullopt;
    }
    logic::ports::OtaStatus stStatus{};
    stStatus.bUpdateAvailable = true;
    stStatus.sVersion = optManifest->version;
    stStatus.sReleaseNotes = optManifest->notes;
    m_optCachedStatus = stStatus;
    return stStatus;
}

bool OtaDriver::downloadAndInstall(const std::string& sVersion) {
    if (!m_optCachedStatus.has_value()) {
        const auto optStatus = checkForUpdate();
        if (!optStatus.has_value()) {
            return false;
        }
    }
    if (m_optCachedStatus->sVersion != sVersion) {
        return false;
    }
    // In production this would stream the firmware to the OTA subsystem.
    return true;
}

}  // namespace driver
