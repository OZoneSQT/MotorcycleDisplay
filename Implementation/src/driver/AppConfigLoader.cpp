#include "driver/AppConfigLoader.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>

namespace {
std::string trim(const std::string& sValue) {
    const auto iStart = sValue.find_first_not_of(" \t\r\n");
    if (iStart == std::string::npos) {
        return {};
    }
    const auto iEnd = sValue.find_last_not_of(" \t\r\n");
    return sValue.substr(iStart, iEnd - iStart + 1U);
}

std::string toLower(std::string sValue) {
    std::transform(sValue.begin(), sValue.end(), sValue.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return sValue;
}

}  // namespace

namespace driver {

AppConfigLoadResult loadAppConfigFromEnv(const std::string& sEnvFile, const logic::entities::AppMetadata& stDefaults) {
    AppConfigLoadResult stResult{};
    stResult.stConfig = stDefaults;

    std::ifstream ifs(sEnvFile);
    if (!ifs.is_open()) {
        return stResult;
    }

    stResult.bFileFound = true;

    std::string sLine;
    while (std::getline(ifs, sLine)) {
        if (!sLine.empty() && sLine.back() == '\r') {
            sLine.pop_back();
        }
        const auto sTrimmed = trim(sLine);
        if (sTrimmed.empty() || sTrimmed.front() == '#') {
            continue;
        }
        const auto iEquals = sTrimmed.find('=');
        if (iEquals == std::string::npos) {
            stResult.vWarnings.push_back("Ignoring malformed line: " + sTrimmed);
            continue;
        }
        const auto sKey = toLower(trim(sTrimmed.substr(0U, iEquals)));
        const auto sValue = trim(sTrimmed.substr(iEquals + 1U));
        if (sKey.empty()) {
            stResult.vWarnings.push_back("Ignoring entry with empty key");
            continue;
        }
        stResult.mapEntries[sKey] = sValue;

        if (sKey == "version") {
            stResult.stConfig.sVersion = sValue;
        } else if (sKey == "build_id") {
            stResult.stConfig.sBuildId = sValue;
        } else if (sKey == "app_name") {
            stResult.stConfig.sAppName = sValue;
        } else if (sKey == "copyright") {
            stResult.stConfig.sCopyright = sValue;
        } else if (sKey == "contact_info") {
            stResult.stConfig.sContactInfo = sValue;
        } else if (sKey == "ota_source_link") {
            stResult.stConfig.sOtaSourceLink = sValue;
        } else if (sKey == "debug") {
            const auto sNormalized = toLower(sValue);
            if (sNormalized == "debug_true" || sNormalized == "true" || sNormalized == "1") {
                stResult.stConfig.bDebugEnabled = true;
            } else if (sNormalized == "debug_false" || sNormalized == "false" || sNormalized == "0") {
                stResult.stConfig.bDebugEnabled = false;
            } else {
                stResult.vWarnings.push_back("Unrecognized debug value: " + sValue);
            }
        }
    }

    return stResult;
}

}  // namespace driver
