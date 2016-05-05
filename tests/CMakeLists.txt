#
# Tests
#

enable_testing()

# Build Catch unit tests
#function(add_catch_test _testname)
#	add_executable(${_testname} ${_testname}.cpp)
#	target_link_libraries(${_testname} Catch)
#	add_test(NAME test_${_testname} COMMAND ${_testname})
#endfunction()
#
#file(GLOB catch_tests LIST_DIRECTORIES false *.cpp)
#foreach(catch_test IN LIST catch_tests)
#	add_catch_test(${catch_test})
#endforeach()

# Ensure headers include their own dependencies
add_subdirectory(header_dependencies)

