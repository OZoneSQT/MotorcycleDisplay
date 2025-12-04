#pragma once

#include <cstdint>
#include <mutex>
#include <queue>

#include "logic/ports/ICanPort.hpp"

namespace driver {

struct CanConfig {
    std::uint32_t u32BaudRate{500000U};
    std::uint32_t u32TxPin{5U};
    std::uint32_t u32RxPin{4U};
};

class CanDriver : public logic::ports::ICanPort {
public:
    explicit CanDriver(CanConfig stConfig) : m_stConfig{stConfig} {}
    ~CanDriver() override = default;

    bool initialize() override;
    std::optional<logic::ports::RawCanFrame> readFrame() override;
    void enqueueFrame(const logic::ports::RawCanFrame& stFrame);

protected:
    [[nodiscard]] const CanConfig& stConfig() const noexcept { return m_stConfig; }

private:
    CanConfig m_stConfig;
    bool m_bInitialized{false};
    std::queue<logic::ports::RawCanFrame> m_qFifo;
    std::mutex m_mtxMutex;
};

}  // namespace driver
