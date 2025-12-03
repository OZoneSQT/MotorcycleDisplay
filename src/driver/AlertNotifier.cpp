#include "driver/AlertNotifier.hpp"

#include <utility>

namespace driver {

AlertNotifier::AlertNotifier(Callback fnCallback) : m_fnCallback{std::move(fnCallback)} {}

void AlertNotifier::onAlertChanged(const logic::entities::AlertState& stState) {
    if (m_fnCallback) {
        m_fnCallback(stState);
    }
}

}  // namespace driver
