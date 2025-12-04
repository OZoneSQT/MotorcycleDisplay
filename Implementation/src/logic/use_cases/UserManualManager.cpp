#include "logic/use_cases/UserManualManager.hpp"

#include <optional>
#include <string>
#include <utility>

namespace logic::use_cases {

UserManualManager::UserManualManager(logic::ports::IStoragePort& rStoragePort) : m_rStoragePort{rStoragePort} {}

namespace {
std::string sParentDirectory(const std::string& sPath) {
    const auto pos = sPath.find_last_of("/\\");
    if (pos == std::string::npos) {
        return {};
    }
    return sPath.substr(0U, pos);
}
}  // namespace

bool UserManualManager::bSetManualPath(std::string sPath) {
    m_sManualPath = std::move(sPath);
    if (m_sManualPath.empty()) {
        return false;
    }
    const auto sDirectory = sParentDirectory(m_sManualPath);
    return sDirectory.empty() ? true : m_rStoragePort.ensureDirectory(sDirectory);
}

std::optional<std::string> UserManualManager::optLoadManual() const {
    if (m_sManualPath.empty()) {
        return std::nullopt;
    }
    const auto optContent = m_rStoragePort.readText(m_sManualPath);
    if (!optContent.has_value() || optContent->empty()) {
        return std::nullopt;
    }
    return optContent;
}

}  // namespace logic::use_cases
