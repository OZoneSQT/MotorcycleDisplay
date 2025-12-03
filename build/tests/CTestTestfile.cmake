# CMake generated Testfile for 
# Source directory: C:/GitHub/MotorcycleDisplay/tests
# Build directory: C:/GitHub/MotorcycleDisplay/build/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test([=[motorcycle_tests]=] "C:/GitHub/MotorcycleDisplay/build/tests/Debug/motorcycle_tests.exe")
  set_tests_properties([=[motorcycle_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/GitHub/MotorcycleDisplay/tests/CMakeLists.txt;25;add_test;C:/GitHub/MotorcycleDisplay/tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test([=[motorcycle_tests]=] "C:/GitHub/MotorcycleDisplay/build/tests/Release/motorcycle_tests.exe")
  set_tests_properties([=[motorcycle_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/GitHub/MotorcycleDisplay/tests/CMakeLists.txt;25;add_test;C:/GitHub/MotorcycleDisplay/tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test([=[motorcycle_tests]=] "C:/GitHub/MotorcycleDisplay/build/tests/MinSizeRel/motorcycle_tests.exe")
  set_tests_properties([=[motorcycle_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/GitHub/MotorcycleDisplay/tests/CMakeLists.txt;25;add_test;C:/GitHub/MotorcycleDisplay/tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test([=[motorcycle_tests]=] "C:/GitHub/MotorcycleDisplay/build/tests/RelWithDebInfo/motorcycle_tests.exe")
  set_tests_properties([=[motorcycle_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/GitHub/MotorcycleDisplay/tests/CMakeLists.txt;25;add_test;C:/GitHub/MotorcycleDisplay/tests/CMakeLists.txt;0;")
else()
  add_test([=[motorcycle_tests]=] NOT_AVAILABLE)
endif()
