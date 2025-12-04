#include "logic/use_cases/DataLogger.hpp"

#include <array>
#include <iomanip>
#include <sstream>

#include "logic/use_cases/AuditLogger.hpp"

namespace {
constexpr const char* kDefaultFilename = "vehicle_log.csv";

std::string joinPath(const std::string& base, const std::string& name) {
    if (base.empty()) {
        return name;
    }
    const bool trailingSlash = base.back() == '/' || base.back() == '\\';
    return trailingSlash ? base + name : base + "/" + name;
}

}  // namespace

namespace logic::use_cases {

DataLogger::DataLogger(logic::ports::IStoragePort& rStoragePort, std::string sLogDirectory)
    : m_rStoragePort{rStoragePort}, m_sLogDirectory{std::move(sLogDirectory)} {}

bool DataLogger::bInitialize() {
    if (!m_rStoragePort.ensureDirectory(m_sLogDirectory)) {
        if (m_pAuditLogger != nullptr) {
            m_pAuditLogger->bLogSoftwareError("DATA_LOGGER_INIT_FAILED", "Failed to ensure log directory", "DataLogger");
        }
        return false;
    }
    m_sLogFilePath = joinPath(m_sLogDirectory, kDefaultFilename);
    m_bHeaderWritten = false;
    return true;
}

bool DataLogger::bLog(const logic::entities::VehicleData& stData) {
    std::scoped_lock guard{m_mtxLock};
    if (m_sLogFilePath.empty() && !bInitialize()) {
        return false;
    }

    if (!m_bHeaderWritten) {
        const std::vector<std::string> vHeader{
            "timestamp_ms", "speed_kph", "rpm", "throttle_percent", "abs_active", "engine_temp_c", "battery_voltage"};
        if (!m_rStoragePort.appendCsv(m_sLogFilePath, vHeader)) {
            if (m_pAuditLogger != nullptr) {
                m_pAuditLogger->bLogSoftwareError("DATA_LOGGER_HEADER_FAILED", "Failed to write data log header", "DataLogger");
            }
            return false;
        }
        m_bHeaderWritten = true;
    }

    if (!m_rStoragePort.appendCsv(m_sLogFilePath, vToCsvRow(stData))) {
        if (m_pAuditLogger != nullptr) {
            m_pAuditLogger->bLogSoftwareError("DATA_LOGGER_WRITE_FAILED", "Failed to append data log row", "DataLogger");
        }
        return false;
    }
    return true;
}

const std::string& DataLogger::sLogFilePath() const noexcept {
    return m_sLogFilePath;
}

std::vector<std::string> DataLogger::vToCsvRow(const logic::entities::VehicleData& stData) const {
    std::vector<std::string> vRow;
    vRow.reserve(7U);

    auto fnToString = [](auto tValue) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << tValue;
        return oss.str();
    };

    vRow.emplace_back(std::to_string(stData.u64TimestampMs));
    vRow.emplace_back(fnToString(stData.fSpeedKph));
    vRow.emplace_back(fnToString(stData.fEngineRpm));
    vRow.emplace_back(fnToString(stData.fThrottlePercent));
    vRow.emplace_back(stData.bAbsActive ? "1" : "0");
    vRow.emplace_back(fnToString(stData.fEngineTempC));
    vRow.emplace_back(fnToString(stData.fBatteryVoltage));
    return vRow;
}

void DataLogger::setAuditLogger(AuditLogger* pAuditLogger) noexcept {
    m_pAuditLogger = pAuditLogger;
}

}  // namespace logic::use_cases
