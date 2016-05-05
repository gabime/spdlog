#
# Benchmarks against various logging systems
#

#
# Dependencies
#

find_package(Threads)

enable_testing()

# Helper function for building benchmark programs
function(add_benchmark _target)
    set(options "") # no options
    set(singleValueArgs "") # no single-value arguments
    set(multiValueArgs LIBS SOURCES INCLUDES DEFINITIONS) # lists of additional libraries, source files, and include directories
    cmake_parse_arguments(_benchmark "${options}" "${singleValueArgs}" "${multiValueArgs}" ${ARGN})

    add_executable(${_target} ${_target}.cpp ${_benchmark_SOURCES})
    target_include_directories(
        ${_target}
        PUBLIC
        ${HEADER_BASE}
        ${_benchmark_INCLUDES}
    )

    target_link_libraries(
        ${_target}
        ${CMAKE_THREAD_LIBS_INIT}
        ${_benchmark_LIBS}
    )

    if(_benchmark_DEFINITIONS)
        target_compile_definitions(${_target} PUBLIC ${_benchmark_DEFINITIONS})
    endif()

    add_test(NAME test_benchmark_${_target} COMMAND ${_target})
endfunction()

# Benchmark programs
add_benchmark(spdlog-bench)
add_benchmark(spdlog-bench-mt)
add_benchmark(spdlog-async)

if(TARGET zf_log)
    add_benchmark(zf_log-bench LIBS zf_log)
    add_benchmark(zf_log-bench-mt LIBS zf_log)
endif()

find_package(Boost QUIET COMPONENTS log)
if(Boost_FOUND)
    set(BOOST_DEFS "-DBOOST_LOG_DYN_LINK=1")
    add_benchmark(boost-bench LIBS ${Boost_LIBRARIES} INCLUDES ${Boost_INCLUDE_DIRS} DEFINITIONS ${BOOST_DEFS})
    add_benchmark(boost-bench-mt LIBS ${Boost_LIBRARIES} INCLUDES ${Boost_INCLUDE_DIRS} DEFINITIONS ${BOOST_DEFS})
endif()

if(TARGET glog)
    add_benchmark(glog-bench LIBS glog)
    add_benchmark(glog-bench-mt LIBS glog)
endif()

if(TARGET g3logger)
    add_benchmark(g3log-async LIBS g3logger INCLUDES "${g3log_SOURCE_DIR}/src")
endif()

if(TARGET easylogging)
    add_benchmark(easylogging-bench LIBS easylogging)
    add_benchmark(easylogging-bench-mt LIBS easylogging)
endif()

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/logs")

