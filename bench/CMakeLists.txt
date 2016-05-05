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
        target_compile_definitions(${_target} ${_benchmark_DEFINITIONS})
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
    add_benchmark(boost-bench LIBS ${Boost_LIBRARIES} INCLUDES ${Boost_INCLUDE_DIRS})
    add_benchmark(boost-bench-mt LIBS ${Boost_LIBRARIES} INCLUDES ${Boost_INCLUDE_DIRS})
endif()

find_package(glog QUIET)
if(TARGET glog)
    add_benchmark(glog-bench LIBS glog)
    add_benchmark(glog-bench-mt LIBS glog)
endif()

# TODO make g2log find script
# TODO use g2log git submodule
find_package(g2log QUIET)
if(g2log-FOUND)
    set(G2LOG_LIBRARIES lib_g2logger)
    set(G2LOG_INCLUDE_DIRS /home/gabi/devel/g2log/g2log/src)
    add_benchmark(g2log-async LIBS ${G2LOG_LIBRARIES} INCLUDES ${G2LOG_INCLUDE_DIRS})
endif()

if(TARGET easylogging)
    add_benchmark(easylogging-bench LIBS easylogging)
    add_benchmark(easylogging-bench-mt LIBS easylogging)
endif()

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/logs")

