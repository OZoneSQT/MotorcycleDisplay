#pragma once

#include <functional>
#include <optional>

#include "logic/entities/VehicleData.hpp"
#include "logic/ports/ICanPort.hpp"
#include "logic/ports/IClockPort.hpp"

namespace logic::use_cases {

class CanDataProcessor {
public:
    using Decoder = std::function<std::optional<logic::entities::VehicleData>(const logic::ports::RawCanFrame&)>;

    CanDataProcessor(logic::ports::ICanPort& rCanPort, const logic::ports::IClockPort& rClockPort);

    std::optional<logic::entities::VehicleData> optPollOnce();
    void setCustomDecoder(Decoder fnDecoder);

private:
    bool bDecodeKnownFrame(const logic::ports::RawCanFrame& stFrame, logic::entities::VehicleData& stSnapshot) const;

    logic::ports::ICanPort& m_rCanPort;
    const logic::ports::IClockPort& m_rClockPort;
    Decoder m_fnCustomDecoder{};
    mutable logic::entities::VehicleData m_stLastSnapshot{};
    bool m_bInitialized{false};
};

}  // namespace logic::use_cases
