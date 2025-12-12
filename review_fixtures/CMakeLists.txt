cmake_minimum_required(VERSION 3.11)
project(spdlog_review_fixtures CXX)

if(NOT TARGET spdlog)
    find_package(spdlog CONFIG REQUIRED)
endif()

find_package(Threads REQUIRED)

set(SPDLOG_REVIEW_FIXTURES_SOURCES
    fixture_null_deref.cpp
    fixture_use_after_free.cpp
    fixture_out_of_bounds.cpp
    fixture_uninitialized.cpp
    fixture_integer_overflow.cpp
    fixture_leak.cpp
    fixture_data_race.cpp
    fixture_deadlock.cpp
    fixture_format_string_injection.cpp
    fixture_complex_ownership.cpp
    fixture_complex_iterator_invalidation.cpp
    fixture_complex_exception_safety.cpp
    fixture_complex_concurrency.cpp)

foreach(src ${SPDLOG_REVIEW_FIXTURES_SOURCES})
    get_filename_component(target_name ${src} NAME_WE)
    add_executable(${target_name} ${src})
    target_compile_features(${target_name} PRIVATE cxx_std_11)
    target_link_libraries(${target_name} PRIVATE spdlog::spdlog Threads::Threads)
endforeach()
