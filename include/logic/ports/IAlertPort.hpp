#pragma once

#include "logic/entities/Alert.hpp"

namespace logic::ports {

class IAlertPort {
public:
    virtual ~IAlertPort() = default;
    virtual void onAlertChanged(const logic::entities::AlertState& stState) = 0;
};

}  // namespace logic::ports
