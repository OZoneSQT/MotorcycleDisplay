#include <cassert>
#include <filesystem>
#include <fstream>

#include "driver/StorageDriver.hpp"
#include "logic/use_cases/UserManualManager.hpp"

void runUserManualManagerTests() {
    const std::filesystem::path pathRoot{"tests/output/manual"};
    std::filesystem::remove_all(pathRoot);
    std::filesystem::create_directories(pathRoot / "manual");

    const std::string sManualContent =
        "---topic---\n"
        "path: root\n"
        "title: Root Manual\n"
        "body:\n"
        "Root body text.\n"
        "---end---\n"
        "---topic---\n"
        "path: root/topic-one\n"
        "title: Topic One\n"
        "body:\n"
        "Topic one details.\n"
        "---end---\n";

    const auto pathManual = pathRoot / "manual" / "test_manual.menu";
    {
        std::ofstream ofs(pathManual);
        ofs << sManualContent;
    }

    driver::StorageDriver storage(pathRoot.string());
    logic::use_cases::UserManualManager manager(storage);
    assert(manager.bSetManualPath("manual/test_manual.menu"));
    const auto optRoot = manager.optLoadManualTree();
    assert(optRoot.has_value());
    assert(optRoot->sTitle == "Root Manual");
    assert(optRoot->vChildren.size() == 1U);
    assert(optRoot->vChildren.front().sId == "topic-one");
    assert(optRoot->vChildren.front().sTitle == "Topic One");
    assert(optRoot->vChildren.front().vChildren.empty());
}
