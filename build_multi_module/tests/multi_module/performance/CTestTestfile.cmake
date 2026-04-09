# CMake generated Testfile for 
# Source directory: D:/workspace/兼职项目/4.Solo Coder/spdlog/tests/multi_module/performance
# Build directory: D:/workspace/兼职项目/4.Solo Coder/spdlog/build_multi_module/tests/multi_module/performance
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(spdlog-performance-tests "D:/workspace/兼职项目/4.Solo Coder/spdlog/build_multi_module/tests/multi_module/performance/Debug/spdlog-performance-tests.exe" "--order" "decl")
  set_tests_properties(spdlog-performance-tests PROPERTIES  RUN_SERIAL "ON" _BACKTRACE_TRIPLES "D:/workspace/兼职项目/4.Solo Coder/spdlog/tests/multi_module/performance/CMakeLists.txt;34;add_test;D:/workspace/兼职项目/4.Solo Coder/spdlog/tests/multi_module/performance/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(spdlog-performance-tests "D:/workspace/兼职项目/4.Solo Coder/spdlog/build_multi_module/tests/multi_module/performance/Release/spdlog-performance-tests.exe" "--order" "decl")
  set_tests_properties(spdlog-performance-tests PROPERTIES  RUN_SERIAL "ON" _BACKTRACE_TRIPLES "D:/workspace/兼职项目/4.Solo Coder/spdlog/tests/multi_module/performance/CMakeLists.txt;34;add_test;D:/workspace/兼职项目/4.Solo Coder/spdlog/tests/multi_module/performance/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(spdlog-performance-tests "D:/workspace/兼职项目/4.Solo Coder/spdlog/build_multi_module/tests/multi_module/performance/MinSizeRel/spdlog-performance-tests.exe" "--order" "decl")
  set_tests_properties(spdlog-performance-tests PROPERTIES  RUN_SERIAL "ON" _BACKTRACE_TRIPLES "D:/workspace/兼职项目/4.Solo Coder/spdlog/tests/multi_module/performance/CMakeLists.txt;34;add_test;D:/workspace/兼职项目/4.Solo Coder/spdlog/tests/multi_module/performance/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(spdlog-performance-tests "D:/workspace/兼职项目/4.Solo Coder/spdlog/build_multi_module/tests/multi_module/performance/RelWithDebInfo/spdlog-performance-tests.exe" "--order" "decl")
  set_tests_properties(spdlog-performance-tests PROPERTIES  RUN_SERIAL "ON" _BACKTRACE_TRIPLES "D:/workspace/兼职项目/4.Solo Coder/spdlog/tests/multi_module/performance/CMakeLists.txt;34;add_test;D:/workspace/兼职项目/4.Solo Coder/spdlog/tests/multi_module/performance/CMakeLists.txt;0;")
else()
  add_test(spdlog-performance-tests NOT_AVAILABLE)
endif()
