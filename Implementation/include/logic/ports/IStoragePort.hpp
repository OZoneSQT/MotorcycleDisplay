#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace logic::ports {

class IStoragePort {
public:
    virtual ~IStoragePort() = default;
    virtual bool appendCsv(const std::string& sPath, const std::vector<std::string>& vsRow) = 0;
    virtual std::optional<std::string> readText(const std::string& sPath) = 0;
    virtual bool ensureDirectory(const std::string& sPath) = 0;
};

}  // namespace logic::ports
