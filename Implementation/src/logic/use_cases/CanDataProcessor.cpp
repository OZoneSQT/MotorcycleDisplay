#include "logic/use_cases/CanDataProcessor.hpp"

#include <array>
#include <cmath>

#include "logic/use_cases/AuditLogger.hpp"

namespace {
constexpr float kInvalidValue = -1.F;

float decodeUnsigned(const logic::ports::RawCanFrame& stFrame, std::uint8_t u8Idx, float fScale) {
    if (stFrame.u8Dlc <= u8Idx) {
        return kInvalidValue;
    }
    return static_cast<float>(stFrame.au8Data[u8Idx]) * fScale;
}

}  // namespace

namespace logic::use_cases {

CanDataProcessor::CanDataProcessor(logic::ports::ICanPort& rCanPort,
                                   const logic::ports::IClockPort& rClockPort,
                                   logic::use_cases::CanIdConfig stCanIds)
    : m_rCanPort{rCanPort}, m_rClockPort{rClockPort}, m_stCanIds{stCanIds} {}

std::optional<logic::entities::VehicleData> CanDataProcessor::optPollOnce() {
    if (!m_bInitialized) {
        if (!m_rCanPort.initialize()) {
            return std::nullopt;
        }
        m_bInitialized = true;
    }

    auto optFrame = m_rCanPort.readFrame();
    if (!optFrame.has_value()) {
        return std::nullopt;
    }

    auto stSnapshot = m_stLastSnapshot;
    bool bUpdated = false;

    auto fnProcess = [&](const logic::ports::RawCanFrame& stRawFrame) {
        if (m_pAuditLogger != nullptr) {
            m_pAuditLogger->bLogCanCommand(stRawFrame, "CanDataProcessor");
        }
        if (m_fnCustomDecoder) {
            auto optDecoded = m_fnCustomDecoder(stRawFrame);
            if (optDecoded.has_value()) {
                stSnapshot = optDecoded.value();
                stSnapshot.u64TimestampMs = stSnapshot.u64TimestampMs != 0ULL ? stSnapshot.u64TimestampMs : m_rClockPort.nowMs();
                bUpdated = true;
            }
        } else {
            if (bDecodeKnownFrame(stRawFrame, stSnapshot)) {
                stSnapshot.u64TimestampMs = stRawFrame.u64TimestampMs != 0ULL ? stRawFrame.u64TimestampMs : m_rClockPort.nowMs();
                bUpdated = true;
            }
        }
    };

    fnProcess(*optFrame);
    while ((optFrame = m_rCanPort.readFrame()).has_value()) {
        fnProcess(*optFrame);
    }

    if (!bUpdated) {
        return std::nullopt;
    }

    if (stSnapshot.u64TimestampMs == 0ULL) {
        stSnapshot.u64TimestampMs = m_rClockPort.nowMs();
    }

    m_stLastSnapshot = stSnapshot;
    return m_stLastSnapshot;
}

void CanDataProcessor::setCustomDecoder(Decoder fnDecoder) {
    m_fnCustomDecoder = std::move(fnDecoder);
}

void CanDataProcessor::setAuditLogger(AuditLogger* pAuditLogger) noexcept {
    m_pAuditLogger = pAuditLogger;
}

bool CanDataProcessor::bDecodeKnownFrame(const logic::ports::RawCanFrame& stFrame,
                                         logic::entities::VehicleData& stSnapshot) const {
    const auto u32Id = stFrame.u32Id;

    if (u32Id == m_stCanIds.u32Speed) {
        const float fSpeed = decodeUnsigned(stFrame, 0U, 1.F);
        if (fSpeed >= 0.F) {
            stSnapshot.fSpeedKph = fSpeed;
        }
    } else if (u32Id == m_stCanIds.u32Rpm) {
        if (stFrame.u8Dlc >= 2U) {
            const auto u16Raw = static_cast<std::uint16_t>((stFrame.au8Data[1] << 8U) | stFrame.au8Data[0]);
            stSnapshot.fEngineRpm = static_cast<float>(u16Raw);
        }
    } else if (u32Id == m_stCanIds.u32Throttle) {
        const float fThrottle = decodeUnsigned(stFrame, 0U, 0.4F);
        if (fThrottle >= 0.F && fThrottle <= 100.F) {
            stSnapshot.fThrottlePercent = fThrottle;
        }
    } else if (u32Id == m_stCanIds.u32Abs) {
        stSnapshot.bAbsActive = stFrame.u8Dlc > 0U && stFrame.au8Data[0] != 0U;
    } else if (u32Id == m_stCanIds.u32EngineTemp) {
        const float fTemp = decodeUnsigned(stFrame, 0U, 1.F) - 40.F;
        if (!std::isnan(fTemp)) {
            stSnapshot.fEngineTempC = fTemp;
        }
    } else if (u32Id == m_stCanIds.u32Battery) {
        const float fVoltage = decodeUnsigned(stFrame, 0U, 0.1F);
        if (fVoltage >= 0.F) {
            stSnapshot.fBatteryVoltage = fVoltage;
        }
    } else {
        return false;
    }

    return stSnapshot.bIsValid();
}

}  // namespace logic::use_cases
