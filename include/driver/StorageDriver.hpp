#pragma once

#include <mutex>
#include <string>

#include "logic/ports/IStoragePort.hpp"

namespace driver {

class StorageDriver : public logic::ports::IStoragePort {
public:
    explicit StorageDriver(std::string sRootPath);

    bool appendCsv(const std::string& sPath, const std::vector<std::string>& vsRow) override;
    std::optional<std::string> readText(const std::string& sPath) override;
    bool ensureDirectory(const std::string& sPath) override;

private:
    std::string m_sRootPath;
    std::mutex m_mtxMutex;
};

}  // namespace driver
