#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "logic/use_cases/CanIds.hpp"

namespace driver {

struct CanIdConfigLoadResult {
    logic::use_cases::CanIdConfig stConfig;
    std::vector<std::string> vWarnings;
    bool bFileFound{false};
    std::unordered_map<std::string, std::string> mapEntries;
};

CanIdConfigLoadResult loadCanIdConfigFromEnv(const std::string& sEnvFile,
                                             const logic::use_cases::CanIdConfig& stDefaults = logic::use_cases::kDefaultCanIds);

}  // namespace driver
