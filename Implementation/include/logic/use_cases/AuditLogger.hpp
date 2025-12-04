#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "logic/entities/AuditEvent.hpp"
#include "logic/ports/IClockPort.hpp"
#include "logic/ports/ICanPort.hpp"
#include "logic/ports/IStoragePort.hpp"

namespace logic::use_cases {

class AuditLogger {
public:
    AuditLogger(logic::ports::IStoragePort& rStoragePort,
                const logic::ports::IClockPort& rClockPort,
                std::string sLogDirectory);

    bool bInitialize();
    bool bLogCanCommand(const logic::ports::RawCanFrame& stFrame, const std::string& sSource);
    bool bLogSoftwareError(const std::string& sErrorCode,
                           const std::string& sMessage,
                           const std::string& sSource);

    [[nodiscard]] const std::string& sLogFilePath() const noexcept;

private:
    bool bLogEvent(const logic::entities::AuditEvent& stEvent);
    std::vector<std::string> vToCsvRow(const logic::entities::AuditEvent& stEvent) const;
    static std::string sFormatCanPayload(const logic::ports::RawCanFrame& stFrame);

    logic::ports::IStoragePort& m_rStoragePort;
    const logic::ports::IClockPort& m_rClockPort;
    std::string m_sLogDirectory;
    std::string m_sLogFilePath;
    std::mutex m_mtxLock;
    bool m_bHeaderWritten{false};
};

}  // namespace logic::use_cases
