#pragma once

#include <string>

namespace logic::entities {

struct AppMetadata {
    std::string sVersion{"0.1.0"};
    std::string sBuildId{"local-dev"};
    std::string sAppName{"Motorcycle Dashboard Simulator"};
    std::string sCopyright{"2025 OZoneSQT"};
    std::string sContactInfo{"support@motorcycledashboard.example"};
    std::string sOtaSourceLink{"https://github.com/OZoneSQT/MotorcycleDisplay/releases/latest/download/ota_manifest.json"};
    bool bDebugEnabled{false};
};

}  // namespace logic::entities
