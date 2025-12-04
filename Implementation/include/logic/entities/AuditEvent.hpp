#pragma once

#include <cstdint>
#include <string>

namespace logic::entities {

enum class AuditEventType {
    kCanCommand,
    kSoftwareError
};

struct AuditEvent {
    std::uint64_t u64TimestampMs{0ULL};
    AuditEventType eType{AuditEventType::kCanCommand};
    std::string sSource{};
    std::string sIdentifier{};
    std::string sPayload{};
    std::string sMessage{};
};

}  // namespace logic::entities
