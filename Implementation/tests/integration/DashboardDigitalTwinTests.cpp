#include <algorithm>
#include <cassert>
#include <cmath>
#include <optional>
#include <string>

#include "logic/entities/Alert.hpp"
#include "simulation/DashboardDigitalTwin.hpp"

namespace {
logic::entities::AlertConfig stMakeSpeedAlert(float fLimit) {
    return {logic::entities::AlertMetadata{"speed_high", "Speed high", logic::entities::AlertSeverity::kWarning},
            logic::entities::AlertThreshold{logic::entities::AlertType::kSpeed, fLimit, true}, std::nullopt};
}

logic::entities::AlertConfig stMakeTempAlert(float fLimit) {
    return {logic::entities::AlertMetadata{"temp_high", "Temp high", logic::entities::AlertSeverity::kCritical},
            logic::entities::AlertThreshold{logic::entities::AlertType::kEngineTemp, fLimit, true}, std::nullopt};
}
}  // namespace

void runDashboardDigitalTwinTests() {
    simulation::DashboardDigitalTwin twin;
    twin.vSetClockNow(1000ULL);
    twin.vSetManualContent("manual/dashboard_manual.md", "Line 1\nLine 2");
    twin.vConfigureAlerts({stMakeSpeedAlert(70.F), stMakeTempAlert(95.F)});

    twin.vEnqueueFrame(simulation::stMakeFrame(0x100U, 1000ULL, {80U}));
    twin.vEnqueueFrame(simulation::stMakeFrame(0x101U, 1000ULL, {0x20U, 0x0FU}));
    twin.vEnqueueFrame(simulation::stMakeFrame(0x102U, 1000ULL, {125U}));
    twin.vEnqueueFrame(simulation::stMakeFrame(0x103U, 1000ULL, {1U}));
    twin.vEnqueueFrame(simulation::stMakeFrame(0x104U, 1000ULL, {140U}));
    twin.vEnqueueFrame(simulation::stMakeFrame(0x105U, 1000ULL, {125U}));

    twin.vProcessOnce();

    const auto& display = twin.rDisplayDriver();
    assert(display.bHasRendered());

    const auto stData = display.stLastData();
    assert(std::fabs(stData.fSpeedKph - 80.F) < 0.1F);
    assert(std::fabs(stData.fThrottlePercent - 50.F) < 0.1F);
    assert(std::fabs(stData.fEngineTempC - 100.F) < 0.1F);
    assert(std::fabs(stData.fBatteryVoltage - 12.5F) < 0.1F);
    assert(stData.bAbsActive);
    assert(stData.u64TimestampMs == 1000ULL);

    const auto vAlerts = display.vLastAlerts();
    assert(!vAlerts.empty());
    const auto bAnySpeedAlert = std::any_of(vAlerts.begin(), vAlerts.end(), [](const auto& stAlert) {
        return stAlert.bActive && stAlert.stMetadata.sId == "speed_high";
    });
    assert(bAnySpeedAlert);

    const auto bAnyTempAlert = std::any_of(vAlerts.begin(), vAlerts.end(), [](const auto& stAlert) {
        return stAlert.bActive && stAlert.stMetadata.sId == "temp_high";
    });
    assert(bAnyTempAlert);

    const auto optManual = display.optLastManual();
    assert(optManual.has_value());
    assert(optManual.value().find("Line 1") != std::string::npos);
}
