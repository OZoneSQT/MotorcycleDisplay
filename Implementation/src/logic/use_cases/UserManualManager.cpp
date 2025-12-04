#include "logic/use_cases/UserManualManager.hpp"

#include <functional>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace logic::use_cases {

UserManualManager::UserManualManager(logic::ports::IStoragePort& rStoragePort) : m_rStoragePort{rStoragePort} {}

namespace {
std::string sTrim(const std::string& sValue) {
    const auto iStart = sValue.find_first_not_of(" \t\r\n");
    if (iStart == std::string::npos) {
        return {};
    }
    const auto iEnd = sValue.find_last_not_of(" \t\r\n");
    return sValue.substr(iStart, iEnd - iStart + 1U);
}

std::string sParentDirectory(const std::string& sPath) {
    const auto pos = sPath.find_last_of("/\\");
    if (pos == std::string::npos) {
        return {};
    }
    return sPath.substr(0U, pos);
}

std::string sParentPath(const std::string& sPath) {
    const auto pos = sPath.find_last_of('/');
    if (pos == std::string::npos) {
        return {};
    }
    return sPath.substr(0U, pos);
}

std::string sLastComponent(const std::string& sPath) {
    const auto pos = sPath.find_last_of('/');
    if (pos == std::string::npos) {
        return sPath;
    }
    return sPath.substr(pos + 1U);
}

struct ManualTopicEntry {
    std::string sPath;
    std::string sTitle;
    std::string sBody;
};

std::optional<std::vector<ManualTopicEntry>> optParseManualEntries(const std::string& sContent) {
    std::vector<ManualTopicEntry> vEntries;
    std::istringstream iss(sContent);
    std::string sLine;
    ManualTopicEntry stCurrent{};
    bool bInTopic = false;
    bool bInBody = false;
    std::ostringstream ossBody;

    auto flushEntry = [&]() {
        if (!bInTopic) {
            return;
        }
        stCurrent.sBody = ossBody.str();
        if (!stCurrent.sBody.empty() && stCurrent.sBody.back() == '\n') {
            stCurrent.sBody.pop_back();
        }
        if (!stCurrent.sPath.empty() && !stCurrent.sTitle.empty()) {
            vEntries.push_back(stCurrent);
        }
        stCurrent = ManualTopicEntry{};
        ossBody.str("");
        ossBody.clear();
        bInTopic = false;
        bInBody = false;
    };

    while (std::getline(iss, sLine)) {
        if (!sLine.empty() && sLine.back() == '\r') {
            sLine.pop_back();
        }
        const auto sTrimmed = sTrim(sLine);
        if (sTrimmed == "---topic---") {
            flushEntry();
            stCurrent = ManualTopicEntry{};
            bInTopic = true;
            bInBody = false;
            ossBody.str("");
            ossBody.clear();
            continue;
        }
        if (sTrimmed == "---end---") {
            flushEntry();
            continue;
        }
        if (!bInTopic) {
            continue;
        }
        if (sTrimmed.rfind("path:", 0U) == 0U) {
            stCurrent.sPath = sTrim(sTrimmed.substr(5U));
            continue;
        }
        if (sTrimmed.rfind("title:", 0U) == 0U) {
            stCurrent.sTitle = sTrim(sTrimmed.substr(6U));
            continue;
        }
        if (sTrimmed.rfind("body:", 0U) == 0U) {
            std::string sInitial = sTrimmed.substr(5U);
            if (!sInitial.empty()) {
                ossBody << sInitial << '\n';
            }
            bInBody = true;
            continue;
        }
        if (bInBody) {
            ossBody << sLine << '\n';
        }
    }
    flushEntry();

    if (vEntries.empty()) {
        return std::nullopt;
    }
    return vEntries;
}

std::optional<logic::entities::ManualNode> optBuildManualTree(const std::vector<ManualTopicEntry>& vEntries) {
    std::unordered_map<std::string, ManualTopicEntry> mapEntries;
    std::unordered_map<std::string, std::vector<std::string>> mapChildren;

    for (const auto& stEntry : vEntries) {
        mapEntries[stEntry.sPath] = stEntry;
        const auto sParent = sParentPath(stEntry.sPath);
        if (!sParent.empty()) {
            mapChildren[sParent].push_back(stEntry.sPath);
        }
    }

    const auto itRoot = mapEntries.find("root");
    if (itRoot == mapEntries.end()) {
        return std::nullopt;
    }

    std::function<logic::entities::ManualNode(const std::string&)> fnBuild = [&](const std::string& sPath) {
        logic::entities::ManualNode stNode{};
        const auto itEntry = mapEntries.find(sPath);
        if (itEntry != mapEntries.end()) {
            stNode.sId = sLastComponent(sPath);
            stNode.sTitle = itEntry->second.sTitle;
            stNode.sBody = itEntry->second.sBody;
        } else {
            stNode.sId = sLastComponent(sPath);
            stNode.sTitle = stNode.sId;
        }

        const auto itChildren = mapChildren.find(sPath);
        if (itChildren != mapChildren.end()) {
            for (const auto& sChildPath : itChildren->second) {
                stNode.vChildren.push_back(fnBuild(sChildPath));
            }
        }
        return stNode;
    };

    return fnBuild("root");
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

std::optional<logic::entities::ManualNode> UserManualManager::optLoadManualTree() const {
    if (m_sManualPath.empty()) {
        return std::nullopt;
    }
    const auto optContent = m_rStoragePort.readText(m_sManualPath);
    if (!optContent.has_value() || optContent->empty()) {
        return std::nullopt;
    }
    if (const auto optEntries = optParseManualEntries(optContent.value()); optEntries.has_value()) {
        if (const auto optRoot = optBuildManualTree(optEntries.value()); optRoot.has_value()) {
            return optRoot;
        }
    }

    logic::entities::ManualNode stFallback{};
    stFallback.sId = "root";
    stFallback.sTitle = "User Manual";
    stFallback.sBody = optContent.value();
    return stFallback;
}

}  // namespace logic::use_cases
