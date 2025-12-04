#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "driver/StorageDriver.hpp"
#include "logic/ports/IClockPort.hpp"
#include "logic/ports/ICanPort.hpp"
#include "logic/use_cases/AuditLogger.hpp"

namespace {
class TestClock : public logic::ports::IClockPort {
public:
    explicit TestClock(std::uint64_t u64NowMs) : m_u64NowMs{u64NowMs} {}

    std::uint64_t nowMs() const override { return m_u64NowMs; }
    void vSetNow(std::uint64_t u64NowMs) { m_u64NowMs = u64NowMs; }

private:
    std::uint64_t m_u64NowMs{0ULL};
};

std::vector<std::string> vReadLines(const std::filesystem::path& pathFile) {
    std::vector<std::string> vLines;
    std::ifstream ifs(pathFile);
    std::string sLine;
    while (std::getline(ifs, sLine)) {
        vLines.push_back(sLine);
    }
    return vLines;
}
}  // namespace

void runAuditLoggerTests() {
    const std::filesystem::path pathRoot{"tests/output"};
    std::filesystem::remove_all(pathRoot);

    driver::StorageDriver storage(pathRoot.string());
    TestClock clock(1000ULL);
    logic::use_cases::AuditLogger logger(storage, clock, "logs");
    assert(logger.bInitialize());

    logic::ports::RawCanFrame frame{};
    frame.u32Id = 0x123U;
    frame.u8Dlc = 2U;
    frame.au8Data[0] = 0xAAU;
    frame.au8Data[1] = 0xBBU;
    frame.u64TimestampMs = 2000ULL;
    assert(logger.bLogCanCommand(frame, "unit_test"));

    clock.vSetNow(4000ULL);
    assert(logger.bLogSoftwareError("E001", "Example failure", "unit_test"));

    const auto pathLog = pathRoot / "logs" / "audit_log.csv";
    assert(std::filesystem::exists(pathLog));

    const auto vLines = vReadLines(pathLog);
    assert(vLines.size() == 3U);
    assert(vLines[0] == "timestamp_ms,event_type,source,identifier,payload,message");
    assert(vLines[1] == "2000,CAN_COMMAND,unit_test,0x00000123,AA BB,dlc=2");
    assert(vLines[2] == "4000,SOFTWARE_ERROR,unit_test,E001,,Example failure");
}
