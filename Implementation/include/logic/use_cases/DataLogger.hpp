#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "logic/entities/VehicleData.hpp"
#include "logic/ports/IStoragePort.hpp"

namespace logic::use_cases {

class AuditLogger;

class DataLogger {
public:
    DataLogger(logic::ports::IStoragePort& rStoragePort, std::string sLogDirectory);

    bool bInitialize();
    bool bLog(const logic::entities::VehicleData& stData);
    [[nodiscard]] const std::string& sLogFilePath() const noexcept;
    void setAuditLogger(AuditLogger* pAuditLogger) noexcept;

private:
    std::vector<std::string> vToCsvRow(const logic::entities::VehicleData& stData) const;

    logic::ports::IStoragePort& m_rStoragePort;
    std::string m_sLogDirectory;
    std::string m_sLogFilePath;
    std::mutex m_mtxLock;
    bool m_bHeaderWritten{false};
    AuditLogger* m_pAuditLogger{nullptr};
};

}  // namespace logic::use_cases
