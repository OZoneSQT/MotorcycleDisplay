#include <cassert>
#include <filesystem>

#include "driver/StorageDriver.hpp"
#include "logic/entities/VehicleData.hpp"
#include "logic/use_cases/DataLogger.hpp"

void runDataLoggerTests() {
    const std::string root = "tests/output";
    std::filesystem::remove_all(root);
    driver::StorageDriver storage(root);
    logic::use_cases::DataLogger logger(storage, "logs");
    assert(logger.bInitialize());

    logic::entities::VehicleData sample;
    sample.u64TimestampMs = 123U;
    sample.fSpeedKph = 50.F;
    sample.fEngineRpm = 2500.F;
    sample.fThrottlePercent = 30.F;
    sample.fEngineTempC = 95.F;
    sample.fBatteryVoltage = 12.4F;

    assert(logger.bLog(sample));
    assert(std::filesystem::exists(root + "/logs/vehicle_log.csv"));
}
