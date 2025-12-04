#include "driver/ConsoleDisplayDriver.hpp"

#include <iomanip>
#include <iostream>

namespace driver {

void ConsoleDisplayDriver::initialize() {
    // No-op for console output.
}

void ConsoleDisplayDriver::drawDashboard(const logic::entities::VehicleData& stData,
                                         const std::vector<logic::entities::AlertState>& vAlerts,
                                         const std::optional<logic::entities::ManualPanel>& optManualPanel) {
    std::scoped_lock guard{m_mutex};
    std::cout << "\n=== Dashboard ===\n";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "Speed: " << stData.fSpeedKph << " km/h\n";
    std::cout << "RPM: " << stData.fEngineRpm << "\n";
    std::cout << "Throttle: " << stData.fThrottlePercent << "%\n";
    std::cout << "ABS: " << (stData.bAbsActive ? "Active" : "Ready") << "\n";
    std::cout << "Engine Temp: " << stData.fEngineTempC << " C\n";
    std::cout << "Battery: " << stData.fBatteryVoltage << " V\n";
    auto severityToString = [](logic::entities::AlertSeverity severity) {
        switch (severity) {
            case logic::entities::AlertSeverity::kInfo:
                return "INFO";
            case logic::entities::AlertSeverity::kWarning:
                return "WARN";
            case logic::entities::AlertSeverity::kCritical:
                return "CRIT";
            default:
                return "UNK";
        }
    };

    std::cout << "Alerts: " << vAlerts.size() << "\n";
    for (const auto& stAlert : vAlerts) {
        if (!stAlert.bActive) {
            continue;
        }
        std::cout << " - [" << severityToString(stAlert.stMetadata.eSeverity) << "] " << stAlert.stMetadata.sMessage
                  << "\n";
    }
    if (optManualPanel.has_value()) {
        const auto& stPanel = optManualPanel.value();
        std::cout << "\n[User Manual]\n";
        std::cout << stPanel.sTitle << "\n";
        if (!stPanel.sBody.empty()) {
            std::cout << stPanel.sBody << "\n";
        }
        if (!stPanel.vChildren.empty()) {
            std::cout << "\nTopics:\n";
            std::size_t uIndex = 1U;
            for (const auto& stItem : stPanel.vChildren) {
                std::cout << " " << uIndex++ << ". [" << stItem.sId << "] " << stItem.sTitle << "\n";
            }
        }
        std::cout << "\n[Navigation] Home=" << (stPanel.bCanGoHome ? "Yes" : "No")
                  << " | Back=" << (stPanel.bCanGoBack ? "Yes" : "No") << "\n";
    }
    std::cout << std::flush;
}

}  // namespace driver
