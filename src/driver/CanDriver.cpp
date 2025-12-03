#include "driver/CanDriver.hpp"

namespace driver {

bool CanDriver::initialize() {
    m_bInitialized = true;
    return true;
}

std::optional<logic::ports::RawCanFrame> CanDriver::readFrame() {
    std::scoped_lock guard{m_mtxMutex};
    if (!m_bInitialized || m_qFifo.empty()) {
        return std::nullopt;
    }
    auto stFrame = m_qFifo.front();
    m_qFifo.pop();
    return stFrame;
}

void CanDriver::enqueueFrame(const logic::ports::RawCanFrame& stFrame) {
    std::scoped_lock guard{m_mtxMutex};
    m_qFifo.push(stFrame);
}

}  // namespace driver
