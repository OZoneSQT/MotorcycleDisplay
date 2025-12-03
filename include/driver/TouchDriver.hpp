#pragma once

#include <optional>
#include <string>

namespace driver {

struct TouchEvent {
    enum class Type { kTap, kSwipeLeft, kSwipeRight };
    Type eType{Type::kTap};
};

class TouchDriver {
public:
    virtual ~TouchDriver() = default;
    virtual std::optional<TouchEvent> optReadEvent() = 0;
};

}  // namespace driver
