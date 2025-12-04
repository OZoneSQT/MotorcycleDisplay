#include <exception>
#include <iostream>

void runAlertEvaluatorTests();
void runDataLoggerTests();
void runDashboardDigitalTwinTests();

int main() {
    try {
        runAlertEvaluatorTests();
        runDataLoggerTests();
        runDashboardDigitalTwinTests();
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
