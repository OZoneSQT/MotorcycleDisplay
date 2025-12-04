#include <cassert>
#include <filesystem>
#include <fstream>

#include "driver/AppConfigLoader.hpp"

namespace {
std::filesystem::path writeTempEnv(const std::string& sName, const std::string& sContent) {
    const auto path = std::filesystem::temp_directory_path() / sName;
    std::ofstream ofs(path);
    ofs << sContent;
    return path;
}
}  // namespace

void runAppConfigLoaderTests() {
    const auto pathValid = writeTempEnv("app_config_valid.env",
                                        "version=1.2.3\n"
                                        "build_id=build-789\n"
                                        "app_name=Motorcycle HUD\n"
                                        "copyright=2025 MotoCorp\n"
                                        "contact_info=support@moto.example\n"
                                        "ota_source_link=https://updates.example/ota.json\n"
                                        "debug=debug_true\n");

    const auto stValid = driver::loadAppConfigFromEnv(pathValid.string());
    assert(stValid.bFileFound);
    assert(stValid.stConfig.sVersion == "1.2.3");
    assert(stValid.stConfig.sBuildId == "build-789");
    assert(stValid.stConfig.sAppName == "Motorcycle HUD");
    assert(stValid.stConfig.sContactInfo == "support@moto.example");
    assert(stValid.stConfig.sOtaSourceLink == "https://updates.example/ota.json");
    assert(stValid.stConfig.bDebugEnabled);

    const auto pathInvalid = writeTempEnv("app_config_invalid.env", "debug=maybe\nunknown=value\n");
    const auto stInvalid = driver::loadAppConfigFromEnv(pathInvalid.string());
    assert(stInvalid.bFileFound);
    assert(!stInvalid.vWarnings.empty());
    assert(!stInvalid.stConfig.bDebugEnabled);
    assert(stInvalid.mapEntries.at("debug") == "maybe");
    assert(stInvalid.mapEntries.at("unknown") == "value");

    const auto stMissing = driver::loadAppConfigFromEnv("does_not_exist.env");
    assert(!stMissing.bFileFound);

    // Ensure repository .env stays aligned with expected runtime metadata.
    const auto pathRepositoryEnv = std::filesystem::path{PROJECT_SOURCE_DIR} / ".env";
    assert(std::filesystem::exists(pathRepositoryEnv));
    const auto stRepositoryEnv = driver::loadAppConfigFromEnv(pathRepositoryEnv.string());
    assert(stRepositoryEnv.bFileFound);
    assert(stRepositoryEnv.stConfig.sVersion == "0.1.0");
    assert(stRepositoryEnv.stConfig.sBuildId == "local-dev");
    assert(stRepositoryEnv.stConfig.sAppName == "Motorcycle Dashboard Simulator");
    assert(stRepositoryEnv.stConfig.sCopyright == "2025 OZoneSQT");
    assert(stRepositoryEnv.stConfig.sContactInfo == "support@motorcycledashboard.example");
    assert(stRepositoryEnv.stConfig.sOtaSourceLink == "https://github.com/OZoneSQT/MotorcycleDisplay/releases/latest/download/ota_manifest.json");
    assert(!stRepositoryEnv.stConfig.bDebugEnabled);
}
