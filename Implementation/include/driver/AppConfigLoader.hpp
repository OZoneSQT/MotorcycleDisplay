#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "logic/entities/AppMetadata.hpp"

namespace driver {

struct AppConfigLoadResult {
    logic::entities::AppMetadata stConfig{};
    std::vector<std::string> vWarnings;
    bool bFileFound{false};
    std::unordered_map<std::string, std::string> mapEntries;
};

AppConfigLoadResult loadAppConfigFromEnv(const std::string& sEnvFile,
                                         const logic::entities::AppMetadata& stDefaults = logic::entities::AppMetadata{});

}  // namespace driver
