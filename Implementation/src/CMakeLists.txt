set(MOTORCYCLE_CORE_SOURCES
    controller/AlertController.cpp
    controller/DashboardController.cpp
    controller/OtaController.cpp
    driver/AlertNotifier.cpp
    driver/CanIdConfigLoader.cpp
    driver/AppConfigLoader.cpp
    driver/CanDriver.cpp
    driver/ConsoleDisplayDriver.cpp
    driver/DigitalDisplayDriver.cpp
    driver/OtaDriver.cpp
    driver/SimulatedTouchDriver.cpp
    driver/StorageDriver.cpp
    driver/SystemClock.cpp
    logic/use_cases/AlertEvaluator.cpp
    logic/use_cases/AuditLogger.cpp
    logic/use_cases/CanDataProcessor.cpp
    logic/use_cases/DataLogger.cpp
    logic/use_cases/OtaCoordinator.cpp
    logic/use_cases/UserManualManager.cpp
    simulation/DashboardDigitalTwin.cpp
    ui/DashboardView.cpp
)

add_library(motorcycle_core ${MOTORCYCLE_CORE_SOURCES})

target_include_directories(motorcycle_core
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/../include
)

target_compile_features(motorcycle_core PUBLIC cxx_std_23)

target_compile_options(motorcycle_core
    PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /EHsc>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

add_executable(motorcycle_dashboard main.cpp)

target_link_libraries(motorcycle_dashboard PRIVATE motorcycle_core)

target_compile_options(motorcycle_dashboard
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /EHsc>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

add_executable(motorcycle_twin_simulator
    WIN32
    simulation/InteractiveTwinApp.cpp
)

target_link_libraries(motorcycle_twin_simulator PRIVATE motorcycle_core)

target_compile_options(motorcycle_twin_simulator
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /EHsc>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

if (WIN32)
    target_link_libraries(motorcycle_twin_simulator PRIVATE user32 gdi32)
endif()
