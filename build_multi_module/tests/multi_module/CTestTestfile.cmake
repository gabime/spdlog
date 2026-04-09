# CMake generated Testfile for 
# Source directory: D:/workspace/兼职项目/4.Solo Coder/spdlog/tests/multi_module
# Build directory: D:/workspace/兼职项目/4.Solo Coder/spdlog/build_multi_module/tests/multi_module
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(spdlog-multi-module-tests "D:/workspace/兼职项目/4.Solo Coder/spdlog/build_multi_module/tests/multi_module/Debug/spdlog-multi-module-tests.exe" "--order" "decl")
  set_tests_properties(spdlog-multi-module-tests PROPERTIES  RUN_SERIAL "ON" _BACKTRACE_TRIPLES "D:/workspace/兼职项目/4.Solo Coder/spdlog/tests/multi_module/CMakeLists.txt;50;add_test;D:/workspace/兼职项目/4.Solo Coder/spdlog/tests/multi_module/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(spdlog-multi-module-tests "D:/workspace/兼职项目/4.Solo Coder/spdlog/build_multi_module/tests/multi_module/Release/spdlog-multi-module-tests.exe" "--order" "decl")
  set_tests_properties(spdlog-multi-module-tests PROPERTIES  RUN_SERIAL "ON" _BACKTRACE_TRIPLES "D:/workspace/兼职项目/4.Solo Coder/spdlog/tests/multi_module/CMakeLists.txt;50;add_test;D:/workspace/兼职项目/4.Solo Coder/spdlog/tests/multi_module/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(spdlog-multi-module-tests "D:/workspace/兼职项目/4.Solo Coder/spdlog/build_multi_module/tests/multi_module/MinSizeRel/spdlog-multi-module-tests.exe" "--order" "decl")
  set_tests_properties(spdlog-multi-module-tests PROPERTIES  RUN_SERIAL "ON" _BACKTRACE_TRIPLES "D:/workspace/兼职项目/4.Solo Coder/spdlog/tests/multi_module/CMakeLists.txt;50;add_test;D:/workspace/兼职项目/4.Solo Coder/spdlog/tests/multi_module/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(spdlog-multi-module-tests "D:/workspace/兼职项目/4.Solo Coder/spdlog/build_multi_module/tests/multi_module/RelWithDebInfo/spdlog-multi-module-tests.exe" "--order" "decl")
  set_tests_properties(spdlog-multi-module-tests PROPERTIES  RUN_SERIAL "ON" _BACKTRACE_TRIPLES "D:/workspace/兼职项目/4.Solo Coder/spdlog/tests/multi_module/CMakeLists.txt;50;add_test;D:/workspace/兼职项目/4.Solo Coder/spdlog/tests/multi_module/CMakeLists.txt;0;")
else()
  add_test(spdlog-multi-module-tests NOT_AVAILABLE)
endif()
subdirs("../../_deps/catch2-build")
subdirs("module_core")
subdirs("module_network")
subdirs("module_data")
subdirs("performance")
