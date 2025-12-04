#pragma once

#include <cstdint>
#include <initializer_list>
#include <mutex>
#include <optional>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "controller/AlertController.hpp"
#include "controller/DashboardController.hpp"
#include "driver/CanDriver.hpp"
#include "driver/AppConfigLoader.hpp"
#include "driver/CanIdConfigLoader.hpp"
#include "driver/DigitalDisplayDriver.hpp"
#include "driver/SimulatedTouchDriver.hpp"
#include "logic/entities/Alert.hpp"
#include "logic/entities/AppMetadata.hpp"
#include "logic/ports/IAlertPort.hpp"
#include "logic/ports/IClockPort.hpp"
#include "logic/ports/ICanPort.hpp"
#include "logic/ports/IStoragePort.hpp"
#include "logic/use_cases/AlertEvaluator.hpp"
#include "logic/use_cases/AuditLogger.hpp"
#include "logic/use_cases/CanDataProcessor.hpp"
#include "logic/use_cases/DataLogger.hpp"
#include "logic/use_cases/UserManualManager.hpp"
#include "ui/DashboardView.hpp"

namespace simulation {

class TestClock : public logic::ports::IClockPort {
public:
    TestClock() = default;
    explicit TestClock(std::uint64_t u64StartMs) : m_u64NowMs{u64StartMs} {}

    std::uint64_t nowMs() const override { return m_u64NowMs; }
    void vSetNow(std::uint64_t u64NowMs) { m_u64NowMs = u64NowMs; }
    void vAdvance(std::uint64_t u64DeltaMs) { m_u64NowMs += u64DeltaMs; }

private:
    std::uint64_t m_u64NowMs{0ULL};
};

class InMemoryStoragePort : public logic::ports::IStoragePort {
public:
    bool appendCsv(const std::string& sPath, const std::vector<std::string>& vsRow) override;
    std::optional<std::string> readText(const std::string& sPath) override;
    bool ensureDirectory(const std::string& sPath) override;

    void vSetManual(const std::string& sPath, const std::string& sContent);
    [[nodiscard]] std::vector<std::vector<std::string>> vvCsvRows(const std::string& sPath) const;

private:
    mutable std::mutex m_mtxMutex;
    std::unordered_map<std::string, std::vector<std::vector<std::string>>> m_mapCsvData;
    std::unordered_map<std::string, std::string> m_mapManualFiles;
    std::unordered_set<std::string> m_setDirectories;
};

class CollectingAlertPort : public logic::ports::IAlertPort {
public:
    void onAlertChanged(const logic::entities::AlertState& stState) override;
    [[nodiscard]] const std::vector<logic::entities::AlertState>& vStates() const noexcept;

private:
    std::vector<logic::entities::AlertState> m_vStates;
};

class DashboardDigitalTwin {
public:
    struct TwinInputs {
        float fSpeedKph{0.F};
        float fEngineRpm{0.F};
        float fThrottlePercent{0.F};
        bool bAbsActive{false};
        float fEngineTempC{0.F};
        float fBatteryVoltage{12.F};
    };

    DashboardDigitalTwin();
    explicit DashboardDigitalTwin(driver::CanIdConfigLoadResult stConfigResult);

    void vConfigureAlerts(std::vector<logic::entities::AlertConfig> vConfigs);
    void vSetManualContent(const std::string& sPath, const std::string& sContent);
    void vEnqueueFrame(const logic::ports::RawCanFrame& stFrame);
    void vEnqueueTouch(const driver::TouchEvent& stEvent);
    void vApplyInputs(const TwinInputs& stInputs);
    void vProcessOnce();
    void vManualGoHome();
    void vManualGoBack();
    void vManualOpenTopic(const std::string& sTopicId);
    void vManualRefresh();

    void vSetClockNow(std::uint64_t u64NowMs);
    void vAdvanceClock(std::uint64_t u64DeltaMs);

    [[nodiscard]] driver::DigitalDisplayDriver& rDisplayDriver() noexcept;
    [[nodiscard]] const driver::DigitalDisplayDriver& rDisplayDriver() const noexcept;
    [[nodiscard]] const std::vector<logic::entities::AlertState>& vAlertEvents() const noexcept;
    [[nodiscard]] std::vector<std::vector<std::string>> vvLoggedRows(const std::string& sPath) const;
    [[nodiscard]] const logic::entities::AppMetadata& stAppMetadata() const noexcept { return m_stAppMetadata; }

private:
    TestClock m_stClock;
    InMemoryStoragePort m_stStorage;
    CollectingAlertPort m_stAlertPort;

    driver::CanDriver m_drCanDriver;
    driver::DigitalDisplayDriver m_drDisplayDriver;
    driver::SimulatedTouchDriver m_drTouchDriver;

    logic::use_cases::CanIdConfig m_stCanIds;
    logic::use_cases::CanDataProcessor m_ucCanProcessor;
    logic::use_cases::AlertEvaluator m_ucAlertEvaluator;
    logic::use_cases::AuditLogger m_ucAuditLogger;
    logic::entities::AppMetadata m_stAppMetadata;
    logic::use_cases::DataLogger m_ucDataLogger;
    logic::use_cases::UserManualManager m_ucManualManager;

    ui::DashboardView m_uiView;
    controller::DashboardController m_ctrlDashboard;
    controller::AlertController m_ctrlAlert;
    std::uint64_t m_u64LastInputTimestamp{0ULL};
};

logic::ports::RawCanFrame stMakeFrame(std::uint32_t u32Id,
                                      std::uint64_t u64TimestampMs,
                                      std::initializer_list<std::uint8_t> vBytes);

}  // namespace simulation
