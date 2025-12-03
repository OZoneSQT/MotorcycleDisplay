#include "driver/SimulatedTouchDriver.hpp"

namespace driver {

std::optional<TouchEvent> SimulatedTouchDriver::optReadEvent() {
    std::scoped_lock guard{m_mutex};
    if (m_queue.empty()) {
        return std::nullopt;
    }
    auto stEvent = m_queue.front();
    m_queue.pop();
    return stEvent;
}

void SimulatedTouchDriver::enqueue(const TouchEvent& stEvent) {
    std::scoped_lock guard{m_mutex};
    m_queue.push(stEvent);
}

}  // namespace driver
