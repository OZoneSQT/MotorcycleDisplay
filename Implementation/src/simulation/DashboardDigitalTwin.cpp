#include "simulation/DashboardDigitalTwin.hpp"

#include <cstddef>
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

DashboardDigitalTwin::DashboardDigitalTwin()
    : m_drCanDriver({500000U, 5U, 4U}),
      m_ucCanProcessor(m_drCanDriver, m_stClock),
      m_ucAlertEvaluator(m_stClock, m_stAlertPort),
      m_ucDataLogger(m_stStorage, "logs"),
      m_ucManualManager(m_stStorage),
      m_uiView(m_drDisplayDriver, m_drTouchDriver),
      m_ctrlDashboard(m_ucCanProcessor, m_ucAlertEvaluator, m_ucDataLogger, m_ucManualManager, m_uiView),
      m_ctrlAlert(m_ucAlertEvaluator) {
    m_drDisplayDriver.initialize();
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
}

void DashboardDigitalTwin::vProcessOnce() {
    m_ctrlDashboard.processFrame();
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
