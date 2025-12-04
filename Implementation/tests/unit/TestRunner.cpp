#include <exception>
#include <iostream>

void runAlertEvaluatorTests();
void runDataLoggerTests();
void runCanIdConfigLoaderTests();
void runDashboardDigitalTwinTests();
void runAuditLoggerTests();
void runUserManualManagerTests();
void runAppConfigLoaderTests();

int main() {
    try {
        runAlertEvaluatorTests();
        runDataLoggerTests();
        runCanIdConfigLoaderTests();
        runDashboardDigitalTwinTests();
        runAuditLoggerTests();
        runUserManualManagerTests();
        runAppConfigLoaderTests();
    } catch (const std::exception& ex) {
        std::cerr << "Test failure: " << ex.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown test failure" << std::endl;
        return 1;
    }
    std::cout << "All unit tests passed" << std::endl;
    return 0;
}
