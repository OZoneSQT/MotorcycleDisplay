#pragma once

#include <functional>

#include "logic/ports/IAlertPort.hpp"

namespace driver {

class AlertNotifier : public logic::ports::IAlertPort {
public:
    using Callback = std::function<void(const logic::entities::AlertState&)>;

    explicit AlertNotifier(Callback fnCallback);
    void onAlertChanged(const logic::entities::AlertState& stState) override;

private:
    Callback m_fnCallback;
};

}  // namespace driver
