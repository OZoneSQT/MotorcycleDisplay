set(TEST_SOURCES
    unit/AlertEvaluatorTests.cpp
    unit/DataLoggerTests.cpp
    integration/DashboardDigitalTwinTests.cpp
    unit/TestRunner.cpp
)

add_executable(motorcycle_tests ${TEST_SOURCES})

target_link_libraries(motorcycle_tests PRIVATE motorcycle_core)

target_include_directories(motorcycle_tests
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../include
)

target_compile_features(motorcycle_tests PRIVATE cxx_std_20)

target_compile_options(motorcycle_tests
    PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /EHsc>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

add_test(NAME motorcycle_tests COMMAND motorcycle_tests)
