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
    const std::string sManualContent =
        "---topic---\n"
        "path: root\n"
        "title: Dashboard Manual\n"
        "body:\n"
        "Welcome to the Motorcycle Dashboard manual.\n"
        "Use the topics below to navigate.\n"
        "---end---\n"
        "---topic---\n"
        "path: root/safety\n"
        "title: Riding Safety\n"
        "body:\n"
        "Always wear protective gear.\n"
        "---end---\n";
    twin.vSetManualContent("manual/dashboard_manual.menu", sManualContent);
    twin.vConfigureAlerts({stMakeSpeedAlert(70.F), stMakeTempAlert(95.F)});

    simulation::DashboardDigitalTwin::TwinInputs stInputs{};
    stInputs.fSpeedKph = 80.F;
    stInputs.fEngineRpm = 3872.F;
    stInputs.fThrottlePercent = 50.F;
    stInputs.bAbsActive = true;
    stInputs.fEngineTempC = 100.F;
    stInputs.fBatteryVoltage = 12.5F;

    twin.vApplyInputs(stInputs);
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
    assert(optManual->sTitle == "Dashboard Manual");
    assert(optManual->bCanGoHome == false);
    assert(optManual->bCanGoBack == false);
    assert(optManual->vChildren.size() == 1U);
    assert(optManual->vChildren.front().sId == "safety");
    assert(optManual->vChildren.front().sTitle == "Riding Safety");
}
