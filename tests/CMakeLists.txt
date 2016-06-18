#
# Tests
#

enable_testing()

# Build Catch unit tests
add_library(catch INTERFACE)
target_include_directories(catch INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB catch_tests LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

if (CMAKE_COMPILER_IS_GNUCXX)
  set ( CMAKE_CXX_FLAGS "--std=c++11 -pthread")  
  set ( CMAKE_EXE_LIKKER_FLAGS "-pthread")  
endif ()

add_executable(catch_tests ${catch_tests})
target_link_libraries(catch_tests spdlog)
add_test(NAME catch_tests COMMAND catch_tests)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/logs")



