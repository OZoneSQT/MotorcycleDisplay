#include <chrono>
#include <initializer_list>
#include <iostream>
#include <thread>
#include <vector>

#include "controller/AlertController.hpp"
#include "controller/DashboardController.hpp"
#include "controller/OtaController.hpp"
#include "driver/AlertNotifier.hpp"
#include "driver/AppConfigLoader.hpp"
#include "driver/CanIdConfigLoader.hpp"
#include "driver/CanDriver.hpp"
#include "driver/ConsoleDisplayDriver.hpp"
#include "driver/OtaDriver.hpp"
#include "driver/SimulatedTouchDriver.hpp"
#include "driver/StorageDriver.hpp"
#include "driver/SystemClock.hpp"
#include "logic/use_cases/AuditLogger.hpp"
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
    driver::SystemClock drClock;
    driver::StorageDriver drStorageDriver("data");
    logic::use_cases::AuditLogger ucAuditLogger(drStorageDriver, drClock, "logs");
    if (!ucAuditLogger.bInitialize()) {
        std::cerr << "Failed to initialize audit logger" << std::endl;
        return 1;
    }

    const auto stAppConfigResult = driver::loadAppConfigFromEnv(".env");
    if (!stAppConfigResult.bFileFound) {
        ucAuditLogger.bLogSoftwareError("APP_CONFIG_NOT_FOUND", ".env not found; using defaults", "main");
    }
    for (const auto& sWarning : stAppConfigResult.vWarnings) {
        ucAuditLogger.bLogSoftwareError("APP_CONFIG_WARNING", sWarning, "main");
    }
    const auto stAppConfig = stAppConfigResult.stConfig;
    std::cout << stAppConfig.sAppName << " v" << stAppConfig.sVersion << " (" << stAppConfig.sBuildId << ")" << std::endl;
    std::cout << "Contact: " << stAppConfig.sContactInfo << std::endl;
    std::cout << "OTA source: " << stAppConfig.sOtaSourceLink << std::endl;
    if (stAppConfig.bDebugEnabled) {
        std::cout << "[Debug] Verbose logging enabled" << std::endl;
    }

    driver::CanDriver drCanDriver({500000U, 5U, 4U});
    const auto stCanIdResult = driver::loadCanIdConfigFromEnv("bus.env");
    if (!stCanIdResult.bFileFound) {
        ucAuditLogger.bLogSoftwareError("CAN_ID_CONFIG_NOT_FOUND", "bus.env not found; using defaults", "main");
        std::cerr << "[CanIdConfig] bus.env not found; using compiled defaults" << std::endl;
    }
    for (const auto& sWarning : stCanIdResult.vWarnings) {
        ucAuditLogger.bLogSoftwareError("CAN_ID_CONFIG_WARNING", sWarning, "main");
        std::cerr << "[CanIdConfig] " << sWarning << std::endl;
    }
    const auto stCanIds = stCanIdResult.stConfig;

    driver::ConsoleDisplayDriver drDisplayDriver;
    drDisplayDriver.initialize();

    driver::SimulatedTouchDriver drTouchDriver;
    ui::DashboardView uiView(drDisplayDriver, drTouchDriver);

    driver::AlertNotifier drAlertNotifier([](const logic::entities::AlertState& stState) {
        if (stState.bActive) {
            std::cout << "[Alert] " << stState.stMetadata.sMessage << std::endl;
        }
    });

    logic::use_cases::CanDataProcessor ucCanProcessor(drCanDriver, drClock, stCanIds);
    ucCanProcessor.setAuditLogger(&ucAuditLogger);
    logic::use_cases::AlertEvaluator ucAlertEvaluator(drClock, drAlertNotifier);
    logic::use_cases::DataLogger ucDataLogger(drStorageDriver, "logs");
    ucDataLogger.setAuditLogger(&ucAuditLogger);
    if (!ucDataLogger.bInitialize()) {
        std::cerr << "Failed to initialize data logger" << std::endl;
        return 1;
    }
    logic::use_cases::UserManualManager ucManualManager(drStorageDriver);
    ucManualManager.bSetManualPath("manual/dashboard_manual.menu");

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
        const auto stSpeedFrame = stMakeFrame(stCanIds.u32Speed, u64Timestamp, {static_cast<std::uint8_t>(60U + u64Tick * 5U)});
        const auto stRpmFrame = stMakeFrame(stCanIds.u32Rpm, u64Timestamp, {0x40U, static_cast<std::uint8_t>(0x1DU + u64Tick)});
        const auto stThrottleFrame = stMakeFrame(stCanIds.u32Throttle, u64Timestamp, {static_cast<std::uint8_t>(50U + u64Tick)});
        const auto stAbsFrame = stMakeFrame(stCanIds.u32Abs, u64Timestamp, {static_cast<std::uint8_t>(u64Tick % 2U == 0U ? 1U : 0U)});
        const auto stTempFrame = stMakeFrame(stCanIds.u32EngineTemp, u64Timestamp, {static_cast<std::uint8_t>(90U + u64Tick)});
        const auto stBatteryFrame = stMakeFrame(stCanIds.u32Battery, u64Timestamp, {static_cast<std::uint8_t>(120U)});

        drCanDriver.enqueueFrame(stSpeedFrame);
        drCanDriver.enqueueFrame(stRpmFrame);
        drCanDriver.enqueueFrame(stThrottleFrame);
        drCanDriver.enqueueFrame(stAbsFrame);
        drCanDriver.enqueueFrame(stTempFrame);
        drCanDriver.enqueueFrame(stBatteryFrame);

        ucAuditLogger.bLogCanCommand(stSpeedFrame, "main::enqueue");
        ucAuditLogger.bLogCanCommand(stRpmFrame, "main::enqueue");
        ucAuditLogger.bLogCanCommand(stThrottleFrame, "main::enqueue");
        ucAuditLogger.bLogCanCommand(stAbsFrame, "main::enqueue");
        ucAuditLogger.bLogCanCommand(stTempFrame, "main::enqueue");
        ucAuditLogger.bLogCanCommand(stBatteryFrame, "main::enqueue");

        ctrlDashboard.processFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    return 0;
}
