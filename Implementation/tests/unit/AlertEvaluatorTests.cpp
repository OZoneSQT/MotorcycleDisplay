#include <cassert>
#include <vector>

#include "logic/use_cases/AlertEvaluator.hpp"

namespace {
class FixedClock : public logic::ports::IClockPort {
public:
    explicit FixedClock(std::uint64_t now) : now_{now} {}
    std::uint64_t nowMs() const override { return now_; }

private:
    std::uint64_t now_;
};

class CollectingAlertPort : public logic::ports::IAlertPort {
public:
    void onAlertChanged(const logic::entities::AlertState& state) override { states_.push_back(state); }
    std::vector<logic::entities::AlertState> states_;
};

logic::entities::VehicleData buildData(float speed, float temp) {
    logic::entities::VehicleData data;
    data.fSpeedKph = speed;
    data.fEngineTempC = temp;
    data.fEngineRpm = 3000.F;
    data.fThrottlePercent = 50.F;
    data.fBatteryVoltage = 12.5F;
    data.u64TimestampMs = 1U;
    return data;
}

}  // namespace

void runAlertEvaluatorTests() {
    FixedClock clock{100U};
    CollectingAlertPort alertPort;
    logic::use_cases::AlertEvaluator evaluator(clock, alertPort);

    logic::entities::AlertConfig speedConfig{
        logic::entities::AlertMetadata{"speed", "Speed alert", logic::entities::AlertSeverity::kWarning},
        logic::entities::AlertThreshold{logic::entities::AlertType::kSpeed, 80.F, true}, std::nullopt};

    logic::entities::AlertConfig tempConfig{
        logic::entities::AlertMetadata{"temp", "Temperature alert", logic::entities::AlertSeverity::kCritical},
        logic::entities::AlertThreshold{logic::entities::AlertType::kEngineTemp, 105.F, true}, std::nullopt};

    evaluator.configure({speedConfig, tempConfig});

    evaluator.evaluate(buildData(70.F, 90.F));
    assert(alertPort.states_.empty());

    evaluator.evaluate(buildData(90.F, 90.F));
    assert(!alertPort.states_.empty());
    assert(alertPort.states_[0].bActive);

    evaluator.evaluate(buildData(70.F, 90.F));
    assert(alertPort.states_.back().bActive == false);

    evaluator.evaluate(buildData(70.F, 110.F));
    assert(alertPort.states_.back().stMetadata.sId == "temp");
    assert(alertPort.states_.back().bActive);
}
