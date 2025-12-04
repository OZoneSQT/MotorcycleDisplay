#pragma once

#include <functional>
#include <optional>

#include "logic/entities/VehicleData.hpp"
#include "logic/ports/ICanPort.hpp"
#include "logic/ports/IClockPort.hpp"
#include "logic/use_cases/CanIds.hpp"

namespace logic::use_cases {

class AuditLogger;

class CanDataProcessor {
public:
    using Decoder = std::function<std::optional<logic::entities::VehicleData>(const logic::ports::RawCanFrame&)>;

    CanDataProcessor(logic::ports::ICanPort& rCanPort,
                     const logic::ports::IClockPort& rClockPort,
                     logic::use_cases::CanIdConfig stCanIds = logic::use_cases::kDefaultCanIds);

    std::optional<logic::entities::VehicleData> optPollOnce();
    void setCustomDecoder(Decoder fnDecoder);
    void setAuditLogger(AuditLogger* pAuditLogger) noexcept;

private:
    bool bDecodeKnownFrame(const logic::ports::RawCanFrame& stFrame, logic::entities::VehicleData& stSnapshot) const;

    logic::ports::ICanPort& m_rCanPort;
    const logic::ports::IClockPort& m_rClockPort;
    logic::use_cases::CanIdConfig m_stCanIds;
    Decoder m_fnCustomDecoder{};
    mutable logic::entities::VehicleData m_stLastSnapshot{};
    bool m_bInitialized{false};
    AuditLogger* m_pAuditLogger{nullptr};
};

}  // namespace logic::use_cases
