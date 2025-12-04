#include <assert.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>

#include "driver/CanIdConfigLoader.hpp"

namespace {
std::filesystem::path writeTempEnv(const std::string& sName, const std::string& sContent) {
    const auto path = std::filesystem::temp_directory_path() / sName;
    std::ofstream ofs(path);
    ofs << sContent;
    ofs.close();
    return path;
}

std::string trimWhitespace(const std::string& sValue) {
    const auto iFirst = sValue.find_first_not_of(" \t\r\n");
    if (iFirst == std::string::npos) {
        return {};
    }
    const auto iLast = sValue.find_last_not_of(" \t\r\n");
    return sValue.substr(iFirst, iLast - iFirst + 1U);
}

std::unordered_map<std::string, std::string> parseEnvFile(const std::filesystem::path& pathEnv) {
    std::ifstream ifs(pathEnv);
    std::unordered_map<std::string, std::string> mapEntries;
    std::string sLine;
    while (std::getline(ifs, sLine)) {
        const auto sTrimmed = trimWhitespace(sLine);
        if (sTrimmed.empty() || sTrimmed.front() == '#') {
            continue;
        }
        const auto iEquals = sTrimmed.find('=');
        if (iEquals == std::string::npos) {
            continue;
        }
        const auto sKey = trimWhitespace(sTrimmed.substr(0U, iEquals));
        const auto sValue = trimWhitespace(sTrimmed.substr(iEquals + 1U));
        if (!sKey.empty() && !sValue.empty()) {
            mapEntries.emplace(sKey, sValue);
        }
    }
    return mapEntries;
}
}  // namespace

void runCanIdConfigLoaderTests() {
    const auto pathValid = writeTempEnv("can_ids_valid.env",
                                        "# Example overrides\nCAN_ID_SPEED=0x200\nCAN_ID_RPM=0x201\nCAN_ID_THROTTLE=0x202\nCAN_ID_ABS=0x203\nCAN_ID_ENGINE_TEMP=0x204\nCAN_ID_BATTERY=0x205\n");

    const auto stResultValid = driver::loadCanIdConfigFromEnv(pathValid.string());
    assert(stResultValid.stConfig.u32Speed == 0x200U);
    assert(stResultValid.stConfig.u32EngineTemp == 0x204U);
    assert(stResultValid.vWarnings.empty());
    assert(stResultValid.bFileFound);
    assert(stResultValid.mapEntries.at("CAN_ID_SPEED") == "0x200");
    assert(stResultValid.mapEntries.at("CAN_ID_ABS") == "0x203");
    std::filesystem::remove(pathValid);

    const auto pathInvalid = writeTempEnv("can_ids_invalid.env", "CAN_ID_SPEED=not-a-number\nUNKNOWN_KEY=0x300\n");
    const auto stResultInvalid = driver::loadCanIdConfigFromEnv(pathInvalid.string());
    assert(stResultInvalid.stConfig.u32Speed == logic::use_cases::kDefaultCanIds.u32Speed);
    assert(!stResultInvalid.vWarnings.empty());
    assert(stResultInvalid.bFileFound);
    assert(stResultInvalid.mapEntries.at("CAN_ID_SPEED") == "not-a-number");
    assert(stResultInvalid.mapEntries.at("UNKNOWN_KEY") == "0x300");
    std::filesystem::remove(pathInvalid);

    const auto stResultMissing = driver::loadCanIdConfigFromEnv("nonexistent.env");
    assert(stResultMissing.stConfig.u32Rpm == logic::use_cases::kDefaultCanIds.u32Rpm);
    assert(stResultMissing.vWarnings.empty());
    assert(!stResultMissing.bFileFound);
    assert(stResultMissing.mapEntries.empty());

    const auto pathBusEnv = std::filesystem::path(__FILE__).lexically_normal().parent_path().parent_path().parent_path() / "bus.env";
    assert(std::filesystem::exists(pathBusEnv));
    const auto mapExpectedEntries = parseEnvFile(pathBusEnv);
    assert(!mapExpectedEntries.empty());
    const auto stResultBusEnv = driver::loadCanIdConfigFromEnv(pathBusEnv.string());
    assert(stResultBusEnv.bFileFound);
    assert(stResultBusEnv.vWarnings.empty());
    for (const auto& [sKey, sValue] : mapExpectedEntries) {
        assert(stResultBusEnv.mapEntries.at(sKey) == sValue);
    }
}
