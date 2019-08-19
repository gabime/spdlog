# CMake generated Testfile for 
# Source directory: E:/devel/spdlog/tests
# Build directory: E:/devel/spdlog/win64-release/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(spdlog-utests "E:/devel/spdlog/win64-release/tests/Debug/spdlog-utests.exe")
  set_tests_properties(spdlog-utests PROPERTIES  _BACKTRACE_TRIPLES "E:/devel/spdlog/tests/CMakeLists.txt;48;add_test;E:/devel/spdlog/tests/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(spdlog-utests "E:/devel/spdlog/win64-release/tests/Release/spdlog-utests.exe")
  set_tests_properties(spdlog-utests PROPERTIES  _BACKTRACE_TRIPLES "E:/devel/spdlog/tests/CMakeLists.txt;48;add_test;E:/devel/spdlog/tests/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(spdlog-utests "E:/devel/spdlog/win64-release/tests/MinSizeRel/spdlog-utests.exe")
  set_tests_properties(spdlog-utests PROPERTIES  _BACKTRACE_TRIPLES "E:/devel/spdlog/tests/CMakeLists.txt;48;add_test;E:/devel/spdlog/tests/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(spdlog-utests "E:/devel/spdlog/win64-release/tests/RelWithDebInfo/spdlog-utests.exe")
  set_tests_properties(spdlog-utests PROPERTIES  _BACKTRACE_TRIPLES "E:/devel/spdlog/tests/CMakeLists.txt;48;add_test;E:/devel/spdlog/tests/CMakeLists.txt;0;")
else()
  add_test(spdlog-utests NOT_AVAILABLE)
endif()
