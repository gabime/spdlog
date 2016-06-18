#
# Ensure all headers include all dependencies
#

set(IGNORED_HEADERS "")

set(COMMON_TEST_LIBRARIES spdlog)

add_custom_target(header_dependencies)

file(GLOB_RECURSE headers RELATIVE "${HEADER_BASE}" ${HEADER_BASE}/*.h)
set(test_index 0)
foreach(HEADER ${headers})
    # Sample of relevant variables computed here
    # HEADER: details/line_logger_impl.h
    # symbolname: spdlog_details_line_logger_impl

    # Compute symbolname
    string(REPLACE ".h" "" symbolname "${HEADER}")
    string(MAKE_C_IDENTIFIER "${symbolname}" symbolname)

    list(FIND IGNORED_HEADERS "${HEADER}" _index)
    # If we didn't explicitly ignore this and if we built this target
    if(${_index} EQUAL -1)
        #message(STATUS "${HEADER}: '${symbolname}'")

        set(extension cpp)

        # Name the test and output file with a number, to dodge Windows path length limits.
        # Call it header, instead of test, to avoid polluting the 'executable namespace'
        set(test_name "header_${extension}_${test_index}")

        set(source_file "${CMAKE_CURRENT_SOURCE_DIR}/main.${extension}")

        add_executable(${test_name} "${source_file}")
        target_compile_definitions(${test_name} PRIVATE HEADER_TO_TEST="${HEADER}")
        target_include_directories(${test_name}
            PRIVATE
            ${BUILDTREE_HEADER_BASE}
            ${HEADER_BASE})

        set_target_properties(${test_name} PROPERTIES
            FOLDER "Header dependency tests")

        target_link_libraries(${test_name}
            PRIVATE
            ${COMMON_TEST_LIBRARIES}
            ${LIBRARIES_${symbolname}}
            ${LIBRARIES_${libname}})

        add_test(NAME ${test_name}_builds COMMAND ${test_name})
        add_dependencies(header_dependencies ${test_name})

        math(EXPR test_index "${test_index} + 1")
    endif()
endforeach()


