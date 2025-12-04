#include "driver/CanIdConfigLoader.hpp"

#include <cctype>
#include <fstream>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>

namespace {

std::string trimCopy(std::string_view svValue) {
    std::size_t uStart = 0U;
    std::size_t uEnd = svValue.size();
    while (uStart < uEnd && std::isspace(static_cast<unsigned char>(svValue[uStart])) != 0) {
        ++uStart;
    }
    while (uEnd > uStart && std::isspace(static_cast<unsigned char>(svValue[uEnd - 1U])) != 0) {
        --uEnd;
    }
    return std::string{svValue.substr(uStart, uEnd - uStart)};
}

bool parseValue(const std::string& sValue, std::uint32_t& ru32Out) {
    try {
        const unsigned long ulValue = std::stoul(sValue, nullptr, 0);
        if (ulValue > std::numeric_limits<std::uint32_t>::max()) {
            return false;
        }
        ru32Out = static_cast<std::uint32_t>(ulValue);
        return true;
    } catch (...) {
        return false;
    }
}

}  // namespace

namespace driver {

CanIdConfigLoadResult loadCanIdConfigFromEnv(const std::string& sEnvFile,
                                             const logic::use_cases::CanIdConfig& stDefaults) {
    logic::use_cases::CanIdConfig stConfig = stDefaults;
    std::vector<std::string> vWarnings;
    std::unordered_map<std::string, std::string> mapEntries;

    std::ifstream ifEnv{sEnvFile};
    if (!ifEnv.is_open()) {
        return {stConfig, vWarnings, false, mapEntries};
    }

    // TODO: consider extracting a shared helper if more key mappings are added.
    const std::unordered_map<std::string, std::uint32_t logic::use_cases::CanIdConfig::*> mapKeys{
        {"CAN_ID_SPEED", &logic::use_cases::CanIdConfig::u32Speed},
        {"CAN_ID_RPM", &logic::use_cases::CanIdConfig::u32Rpm},
        {"CAN_ID_THROTTLE", &logic::use_cases::CanIdConfig::u32Throttle},
        {"CAN_ID_ABS", &logic::use_cases::CanIdConfig::u32Abs},
        {"CAN_ID_ENGINE_TEMP", &logic::use_cases::CanIdConfig::u32EngineTemp},
        {"CAN_ID_BATTERY", &logic::use_cases::CanIdConfig::u32Battery},
    };

    std::string sLine;
    for (std::size_t uLine = 1U; std::getline(ifEnv, sLine); ++uLine) {
        const std::string sTrimmed = trimCopy(sLine);
        if (sTrimmed.empty() || sTrimmed[0] == '#') {
            continue;
        }

        const auto uPos = sTrimmed.find('=');
        if (uPos == std::string::npos) {
            vWarnings.emplace_back("Line " + std::to_string(uLine) + ": missing '=' delimiter; skipping entry");
            continue;
        }

        const std::string sKey = trimCopy(std::string_view{sTrimmed}.substr(0U, uPos));
        const std::string sValue = trimCopy(std::string_view{sTrimmed}.substr(uPos + 1U));
        if (sKey.empty()) {
            vWarnings.emplace_back("Line " + std::to_string(uLine) + ": empty key; skipping entry");
            continue;
        }
        if (sValue.empty()) {
            vWarnings.emplace_back("Line " + std::to_string(uLine) + ": empty value; skipping entry");
            continue;
        }

        mapEntries[sKey] = sValue;

        const auto itMapping = mapKeys.find(sKey);
        if (itMapping == mapKeys.end()) {
            continue;
        }

        std::uint32_t u32Parsed = 0U;
        if (!parseValue(sValue, u32Parsed)) {
            vWarnings.emplace_back("Line " + std::to_string(uLine) + ": invalid numeric value '" + sValue + "' for key '" + sKey + "'");
            continue;
        }

        stConfig.*(itMapping->second) = u32Parsed;
    }

    return {stConfig, vWarnings, true, std::move(mapEntries)};
}

}  // namespace driver
