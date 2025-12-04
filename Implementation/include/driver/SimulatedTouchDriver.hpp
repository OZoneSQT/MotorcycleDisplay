#pragma once

#include <mutex>
#include <queue>

#include "driver/TouchDriver.hpp"

namespace driver {

class SimulatedTouchDriver : public TouchDriver {
public:
    std::optional<TouchEvent> optReadEvent() override;
    void enqueue(const TouchEvent& stEvent);

private:
    std::queue<TouchEvent> m_queue;
    std::mutex m_mutex;
};

}  // namespace driver
