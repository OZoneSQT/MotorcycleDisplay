#include "logic/use_cases/AuditLogger.hpp"

#include <iomanip>
#include <sstream>
#include <utility>

namespace {
constexpr const char* kAuditFilename = "audit_log.csv";

std::string joinPath(const std::string& sBase, const std::string& sName) {
    if (sBase.empty()) {
        return sName;
    }
    const bool bHasSlash = sBase.back() == '/' || sBase.back() == '\\';
    return bHasSlash ? sBase + sName : sBase + "/" + sName;
}

}  // namespace

namespace logic::use_cases {

AuditLogger::AuditLogger(logic::ports::IStoragePort& rStoragePort,
                         const logic::ports::IClockPort& rClockPort,
                         std::string sLogDirectory)
    : m_rStoragePort{rStoragePort}, m_rClockPort{rClockPort}, m_sLogDirectory{std::move(sLogDirectory)} {}

bool AuditLogger::bInitialize() {
    std::scoped_lock guard{m_mtxLock};
    if (!m_rStoragePort.ensureDirectory(m_sLogDirectory)) {
        return false;
    }
    m_sLogFilePath = joinPath(m_sLogDirectory, kAuditFilename);
    m_bHeaderWritten = false;
    return true;
}

bool AuditLogger::bLogCanCommand(const logic::ports::RawCanFrame& stFrame, const std::string& sSource) {
    logic::entities::AuditEvent stEvent{};
    stEvent.eType = logic::entities::AuditEventType::kCanCommand;
    stEvent.u64TimestampMs = stFrame.u64TimestampMs != 0ULL ? stFrame.u64TimestampMs : m_rClockPort.nowMs();
    stEvent.sSource = sSource;

    std::ostringstream ossIdentifier;
    ossIdentifier << "0x" << std::uppercase << std::hex << std::setw(8) << std::setfill('0') << stFrame.u32Id;
    stEvent.sIdentifier = ossIdentifier.str();

    stEvent.sPayload = sFormatCanPayload(stFrame);
    stEvent.sMessage = "dlc=" + std::to_string(stFrame.u8Dlc);

    return bLogEvent(stEvent);
}

bool AuditLogger::bLogSoftwareError(const std::string& sErrorCode,
                                    const std::string& sMessage,
                                    const std::string& sSource) {
    logic::entities::AuditEvent stEvent{};
    stEvent.eType = logic::entities::AuditEventType::kSoftwareError;
    stEvent.u64TimestampMs = m_rClockPort.nowMs();
    stEvent.sSource = sSource;
    stEvent.sIdentifier = sErrorCode;
    stEvent.sPayload.clear();
    stEvent.sMessage = sMessage;

    return bLogEvent(stEvent);
}

const std::string& AuditLogger::sLogFilePath() const noexcept {
    return m_sLogFilePath;
}

bool AuditLogger::bLogEvent(const logic::entities::AuditEvent& stEvent) {
    std::scoped_lock guard{m_mtxLock};
    if (m_sLogFilePath.empty()) {
        if (!m_rStoragePort.ensureDirectory(m_sLogDirectory)) {
            return false;
        }
        m_sLogFilePath = joinPath(m_sLogDirectory, kAuditFilename);
        m_bHeaderWritten = false;
    }

    if (!m_bHeaderWritten) {
        const std::vector<std::string> vHeader{"timestamp_ms", "event_type", "source", "identifier", "payload", "message"};
        if (!m_rStoragePort.appendCsv(m_sLogFilePath, vHeader)) {
            return false;
        }
        m_bHeaderWritten = true;
    }

    return m_rStoragePort.appendCsv(m_sLogFilePath, vToCsvRow(stEvent));
}

std::vector<std::string> AuditLogger::vToCsvRow(const logic::entities::AuditEvent& stEvent) const {
    std::vector<std::string> vRow;
    vRow.reserve(6U);

    vRow.emplace_back(std::to_string(stEvent.u64TimestampMs));
    vRow.emplace_back(stEvent.eType == logic::entities::AuditEventType::kCanCommand ? "CAN_COMMAND" : "SOFTWARE_ERROR");
    vRow.emplace_back(stEvent.sSource);
    vRow.emplace_back(stEvent.sIdentifier);
    vRow.emplace_back(stEvent.sPayload);
    vRow.emplace_back(stEvent.sMessage);
    return vRow;
}

std::string AuditLogger::sFormatCanPayload(const logic::ports::RawCanFrame& stFrame) {
    if (stFrame.u8Dlc == 0U) {
        return {};
    }

    std::ostringstream oss;
    oss << std::uppercase << std::hex;
    for (std::uint8_t uIndex = 0U; uIndex < stFrame.u8Dlc && uIndex < stFrame.au8Data.size(); ++uIndex) {
        if (uIndex > 0U) {
            oss << ' ';
        }
        oss << std::setw(2) << std::setfill('0') << static_cast<int>(stFrame.au8Data[uIndex]);
    }
    return oss.str();
}

}  // namespace logic::use_cases
