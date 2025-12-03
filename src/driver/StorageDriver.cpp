#include "driver/StorageDriver.hpp"

#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>

namespace {
std::string sJoinPath(const std::string& sRoot, const std::string& sRelative) {
    if (sRelative.empty()) {
        return sRoot;
    }
    if (sRoot.empty()) {
        return sRelative;
    }
    const bool bHasSlash = sRoot.back() == '/' || sRoot.back() == '\\';
    return bHasSlash ? sRoot + sRelative : sRoot + "/" + sRelative;
}

std::string sParentPath(const std::string& sPath) {
    const auto uPos = sPath.find_last_of("/\\");
    if (uPos == std::string::npos) {
        return {};
    }
    return sPath.substr(0U, uPos);
}

}  // namespace

namespace driver {

StorageDriver::StorageDriver(std::string sRootPath) : m_sRootPath{std::move(sRootPath)} {}

bool StorageDriver::appendCsv(const std::string& sPath, const std::vector<std::string>& vsRow) {
    std::scoped_lock guard{m_mtxMutex};
    const auto sDirectory = sParentPath(sPath);
    if (!sDirectory.empty()) {
        ensureDirectory(sDirectory);
    }
    std::ofstream stream{sJoinPath(m_sRootPath, sPath), std::ios::app};
    if (!stream.is_open()) {
        return false;
    }

    for (std::size_t uIndex = 0; uIndex < vsRow.size(); ++uIndex) {
        stream << vsRow[uIndex];
        if (uIndex + 1U < vsRow.size()) {
            stream << ',';
        }
    }
    stream << '\n';
    return true;
}

std::optional<std::string> StorageDriver::readText(const std::string& sPath) {
    std::scoped_lock guard{m_mtxMutex};
    std::ifstream stream{sJoinPath(m_sRootPath, sPath)};
    if (!stream.is_open()) {
        return std::nullopt;
    }
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

bool StorageDriver::ensureDirectory(const std::string& sPath) {
    if (sPath.empty()) {
        return true;
    }
    const auto stFullPath = std::filesystem::path{sJoinPath(m_sRootPath, sPath)};
    std::error_code ec;
    const bool bCreated = std::filesystem::create_directories(stFullPath, ec);
    if (ec && !std::filesystem::exists(stFullPath)) {
        return false;
    }
    return bCreated || std::filesystem::exists(stFullPath);
}

}  // namespace driver
