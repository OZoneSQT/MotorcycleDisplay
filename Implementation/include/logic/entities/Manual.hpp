#pragma once

#include <string>
#include <vector>

namespace logic::entities {

struct ManualNode {
    std::string sId{};
    std::string sTitle{};
    std::string sBody{};
    std::vector<ManualNode> vChildren{};
};

struct ManualMenuEntry {
    std::string sId{};
    std::string sTitle{};
};

struct ManualPanel {
    std::string sTitle{};
    std::string sBody{};
    std::vector<ManualMenuEntry> vChildren{};
    bool bCanGoBack{false};
    bool bCanGoHome{false};
};

}  // namespace logic::entities
