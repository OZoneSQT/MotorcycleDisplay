#include <chrono>
#include <initializer_list>
#include <iostream>
#include <thread>
#include <vector>

#include "controller/AlertController.hpp"
#include "controller/DashboardController.hpp"
#include "controller/OtaController.hpp"
#include "driver/AlertNotifier.hpp"
#include "driver/CanDriver.hpp"
#include "driver/ConsoleDisplayDriver.hpp"
#include "driver/OtaDriver.hpp"
#include "driver/SimulatedTouchDriver.hpp"
#include "driver/StorageDriver.hpp"
#include "driver/SystemClock.hpp"
#include "logic/use_cases/CanDataProcessor.hpp"
#include "logic/use_cases/DataLogger.hpp"
#include "logic/use_cases/OtaCoordinator.hpp"
#include "logic/use_cases/UserManualManager.hpp"
#include "ui/DashboardView.hpp"

namespace {
logic::ports::RawCanFrame stMakeFrame(std::uint32_t u32Id,
                                      std::uint64_t u64Timestamp,
                                      std::initializer_list<std::uint8_t> vPayload) {
    logic::ports::RawCanFrame stFrame{};
    stFrame.u32Id = u32Id;
    stFrame.u64TimestampMs = u64Timestamp;
    stFrame.u8Dlc = static_cast<std::uint8_t>(vPayload.size());
    std::size_t uIndex = 0U;
    for (const auto u8Value : vPayload) {
        stFrame.au8Data[uIndex++] = u8Value;
    }
    return stFrame;
}

}  // namespace

int main() {
    driver::CanDriver drCanDriver({500000U, 5U, 4U});
    driver::SystemClock drClock;
    driver::StorageDriver drStorageDriver("data");
    if (!drStorageDriver.ensureDirectory("logs")) {
        std::cerr << "Failed to create log directory" << std::endl;
        return 1;
    }

    driver::ConsoleDisplayDriver drDisplayDriver;
    drDisplayDriver.initialize();

    driver::SimulatedTouchDriver drTouchDriver;
    ui::DashboardView uiView(drDisplayDriver, drTouchDriver);

    driver::AlertNotifier drAlertNotifier([](const logic::entities::AlertState& stState) {
        if (stState.bActive) {
            std::cout << "[Alert] " << stState.stMetadata.sMessage << std::endl;
        }
    });

    logic::use_cases::CanDataProcessor ucCanProcessor(drCanDriver, drClock);
    logic::use_cases::AlertEvaluator ucAlertEvaluator(drClock, drAlertNotifier);
    logic::use_cases::DataLogger ucDataLogger(drStorageDriver, "logs");
    ucDataLogger.bInitialize();
    logic::use_cases::UserManualManager ucManualManager(drStorageDriver);
    ucManualManager.bSetManualPath("manual/dashboard_manual.md");

    controller::DashboardController ctrlDashboard(ucCanProcessor, ucAlertEvaluator, ucDataLogger, ucManualManager, uiView);
    controller::AlertController ctrlAlert(ucAlertEvaluator);

    std::vector<logic::entities::AlertConfig> vDefaultAlerts{
        {logic::entities::AlertMetadata{"speed_high", "Speed exceeds 120 km/h", logic::entities::AlertSeverity::kWarning},
         logic::entities::AlertThreshold{logic::entities::AlertType::kSpeed, 120.F, true}, std::nullopt},
        {logic::entities::AlertMetadata{"temp_high", "Engine temperature critical", logic::entities::AlertSeverity::kCritical},
         logic::entities::AlertThreshold{logic::entities::AlertType::kEngineTemp, 110.F, true}, std::nullopt}};
    ctrlAlert.updateConfigs(vDefaultAlerts);

    driver::OtaDriver drOtaDriver("data/config/ota_manifest.txt");
    logic::use_cases::OtaCoordinator ucOtaCoordinator(drOtaDriver);
    controller::OtaController ctrlOta(ucOtaCoordinator);

    const auto optOtaStatus = ctrlOta.optCheck();
    if (optOtaStatus.has_value() && optOtaStatus->bUpdateAvailable) {
        std::cout << "OTA update available: v" << optOtaStatus->sVersion << std::endl;
        std::cout << optOtaStatus->sReleaseNotes << std::endl;
    }

    for (std::uint64_t u64Tick = 0ULL; u64Tick < 5ULL; ++u64Tick) {
        const auto u64Timestamp = drClock.nowMs();
        drCanDriver.enqueueFrame(stMakeFrame(0x100U, u64Timestamp, {static_cast<std::uint8_t>(60U + u64Tick * 5U)}));
        drCanDriver.enqueueFrame(stMakeFrame(0x101U, u64Timestamp, {0x40U, static_cast<std::uint8_t>(0x1DU + u64Tick)}));
        drCanDriver.enqueueFrame(stMakeFrame(0x102U, u64Timestamp, {static_cast<std::uint8_t>(50U + u64Tick)}));
        drCanDriver.enqueueFrame(stMakeFrame(0x103U, u64Timestamp, {static_cast<std::uint8_t>(u64Tick % 2U == 0U ? 1U : 0U)}));
        drCanDriver.enqueueFrame(stMakeFrame(0x104U, u64Timestamp, {static_cast<std::uint8_t>(90U + u64Tick)}));
        drCanDriver.enqueueFrame(stMakeFrame(0x105U, u64Timestamp, {static_cast<std::uint8_t>(120U)}));

        ctrlDashboard.processFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    return 0;
}
