#include "simulation/DashboardDigitalTwin.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <utility>

namespace simulation {

bool InMemoryStoragePort::appendCsv(const std::string& sPath, const std::vector<std::string>& vsRow) {
    std::scoped_lock guard{m_mtxMutex};
    auto& vRows = m_mapCsvData[sPath];
    vRows.push_back(vsRow);
    return true;
}

std::optional<std::string> InMemoryStoragePort::readText(const std::string& sPath) {
    std::scoped_lock guard{m_mtxMutex};
    const auto it = m_mapManualFiles.find(sPath);
    if (it == m_mapManualFiles.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool InMemoryStoragePort::ensureDirectory(const std::string& sPath) {
    std::scoped_lock guard{m_mtxMutex};
    m_setDirectories.insert(sPath);
    return true;
}

void InMemoryStoragePort::vSetManual(const std::string& sPath, const std::string& sContent) {
    std::scoped_lock guard{m_mtxMutex};
    m_mapManualFiles[sPath] = sContent;
}

std::vector<std::vector<std::string>> InMemoryStoragePort::vvCsvRows(const std::string& sPath) const {
    std::scoped_lock guard{m_mtxMutex};
    const auto it = m_mapCsvData.find(sPath);
    if (it == m_mapCsvData.end()) {
        return {};
    }
    return it->second;
}

void CollectingAlertPort::onAlertChanged(const logic::entities::AlertState& stState) {
    m_vStates.push_back(stState);
}

const std::vector<logic::entities::AlertState>& CollectingAlertPort::vStates() const noexcept {
    return m_vStates;
}

DashboardDigitalTwin::DashboardDigitalTwin() : DashboardDigitalTwin(driver::loadCanIdConfigFromEnv("bus.env")) {}

DashboardDigitalTwin::DashboardDigitalTwin(driver::CanIdConfigLoadResult stConfigResult)
        : m_drCanDriver({500000U, 5U, 4U}),
            m_stCanIds{stConfigResult.stConfig},
            m_ucCanProcessor(m_drCanDriver, m_stClock, m_stCanIds),
            m_ucAlertEvaluator(m_stClock, m_stAlertPort),
            m_ucAuditLogger(m_stStorage, m_stClock, "logs"),
            m_ucDataLogger(m_stStorage, "logs"),
            m_ucManualManager(m_stStorage),
            m_uiView(m_drDisplayDriver, m_drTouchDriver),
            m_ctrlDashboard(m_ucCanProcessor, m_ucAlertEvaluator, m_ucDataLogger, m_ucManualManager, m_uiView),
            m_ctrlAlert(m_ucAlertEvaluator) {
        m_ucAuditLogger.bInitialize();
        const auto stAppConfigResult = driver::loadAppConfigFromEnv(".env");
        if (!stAppConfigResult.bFileFound) {
            m_ucAuditLogger.bLogSoftwareError("TWIN_APP_CONFIG_NOT_FOUND", ".env not found; using defaults", "DashboardDigitalTwin");
        }
        for (const auto& sWarning : stAppConfigResult.vWarnings) {
            m_ucAuditLogger.bLogSoftwareError("TWIN_APP_CONFIG_WARNING", sWarning, "DashboardDigitalTwin");
        }
        m_stAppMetadata = stAppConfigResult.stConfig;
        for (const auto& sWarning : stConfigResult.vWarnings) {
            std::cerr << "[TwinConfig] " << sWarning << std::endl;
            m_ucAuditLogger.bLogSoftwareError("TWIN_CAN_ID_WARNING", sWarning, "DashboardDigitalTwin");
        }
        m_drDisplayDriver.initialize();
        m_ucCanProcessor.setAuditLogger(&m_ucAuditLogger);
        m_ucDataLogger.setAuditLogger(&m_ucAuditLogger);
        m_ucDataLogger.bInitialize();
}

void DashboardDigitalTwin::vConfigureAlerts(std::vector<logic::entities::AlertConfig> vConfigs) {
    m_ctrlAlert.updateConfigs(std::move(vConfigs));
}

void DashboardDigitalTwin::vSetManualContent(const std::string& sPath, const std::string& sContent) {
    m_stStorage.vSetManual(sPath, sContent);
    m_ucManualManager.bSetManualPath(sPath);
}

void DashboardDigitalTwin::vEnqueueFrame(const logic::ports::RawCanFrame& stFrame) {
    m_drCanDriver.enqueueFrame(stFrame);
    m_ucAuditLogger.bLogCanCommand(stFrame, "DashboardDigitalTwin::enqueue");
}

void DashboardDigitalTwin::vEnqueueTouch(const driver::TouchEvent& stEvent) {
    m_drTouchDriver.enqueue(stEvent);
}

namespace {
std::uint8_t u8ClampToByte(float fValue) {
    const auto fClamped = std::clamp(fValue, 0.F, 255.F);
    return static_cast<std::uint8_t>(std::lround(fClamped));
}

std::uint16_t u16ClampToWord(float fValue) {
    const auto fClamped = std::clamp(fValue, 0.F, 65535.F);
    return static_cast<std::uint16_t>(std::lround(fClamped));
}
}  // namespace

void DashboardDigitalTwin::vApplyInputs(const TwinInputs& stInputs) {
    m_u64LastInputTimestamp = m_stClock.nowMs();

    const auto u8Speed = u8ClampToByte(stInputs.fSpeedKph);
    vEnqueueFrame(stMakeFrame(m_stCanIds.u32Speed, m_u64LastInputTimestamp, {u8Speed}));

    const auto u16Rpm = u16ClampToWord(stInputs.fEngineRpm);
    vEnqueueFrame(stMakeFrame(m_stCanIds.u32Rpm, m_u64LastInputTimestamp,
                              {static_cast<std::uint8_t>(u16Rpm & 0xFFU), static_cast<std::uint8_t>((u16Rpm >> 8U) & 0xFFU)}));

    const auto u8Throttle = u8ClampToByte(stInputs.fThrottlePercent / 0.4F);
    vEnqueueFrame(stMakeFrame(m_stCanIds.u32Throttle, m_u64LastInputTimestamp, {u8Throttle}));

    vEnqueueFrame(stMakeFrame(m_stCanIds.u32Abs, m_u64LastInputTimestamp, {static_cast<std::uint8_t>(stInputs.bAbsActive ? 1U : 0U)}));

    const auto u8Temp = u8ClampToByte(stInputs.fEngineTempC + 40.F);
    vEnqueueFrame(stMakeFrame(m_stCanIds.u32EngineTemp, m_u64LastInputTimestamp, {u8Temp}));

    const auto u8Battery = u8ClampToByte(stInputs.fBatteryVoltage * 10.F);
    vEnqueueFrame(stMakeFrame(m_stCanIds.u32Battery, m_u64LastInputTimestamp, {u8Battery}));
}

void DashboardDigitalTwin::vProcessOnce() {
    m_ctrlDashboard.processFrame();
}

void DashboardDigitalTwin::vManualGoHome() {
    m_ctrlDashboard.goHomeManual();
    m_ctrlDashboard.refreshManualView();
}

void DashboardDigitalTwin::vManualGoBack() {
    m_ctrlDashboard.goBackManual();
    m_ctrlDashboard.refreshManualView();
}

void DashboardDigitalTwin::vManualOpenTopic(const std::string& sTopicId) {
    m_ctrlDashboard.openManualTopic(sTopicId);
    m_ctrlDashboard.refreshManualView();
}

void DashboardDigitalTwin::vManualRefresh() {
    m_ctrlDashboard.refreshManualView();
}

void DashboardDigitalTwin::vSetClockNow(std::uint64_t u64NowMs) {
    m_stClock.vSetNow(u64NowMs);
}

void DashboardDigitalTwin::vAdvanceClock(std::uint64_t u64DeltaMs) {
    m_stClock.vAdvance(u64DeltaMs);
}

driver::DigitalDisplayDriver& DashboardDigitalTwin::rDisplayDriver() noexcept {
    return m_drDisplayDriver;
}

const driver::DigitalDisplayDriver& DashboardDigitalTwin::rDisplayDriver() const noexcept {
    return m_drDisplayDriver;
}

const std::vector<logic::entities::AlertState>& DashboardDigitalTwin::vAlertEvents() const noexcept {
    return m_stAlertPort.vStates();
}

std::vector<std::vector<std::string>> DashboardDigitalTwin::vvLoggedRows(const std::string& sPath) const {
    return m_stStorage.vvCsvRows(sPath);
}

logic::ports::RawCanFrame stMakeFrame(std::uint32_t u32Id,
                                      std::uint64_t u64TimestampMs,
                                      std::initializer_list<std::uint8_t> vBytes) {
    logic::ports::RawCanFrame stFrame{};
    stFrame.u32Id = u32Id;
    stFrame.u64TimestampMs = u64TimestampMs;
    stFrame.u8Dlc = static_cast<std::uint8_t>(vBytes.size());
    std::size_t uIndex = 0U;
    for (const auto u8Value : vBytes) {
        if (uIndex < stFrame.au8Data.size()) {
            stFrame.au8Data[uIndex++] = u8Value;
        }
    }
    return stFrame;
}

}  // namespace simulation
