function(run_checked description)
    execute_process(COMMAND ${ARGN} RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${description} failed (${result}):\n${output}\n${error}")
    endif()
endfunction()

function(configure_checked description source_dir build_dir)
    file(MAKE_DIRECTORY "${build_dir}")
    execute_process(
        COMMAND "${CMAKE_COMMAND}" ${ARGN} "${source_dir}"
        WORKING_DIRECTORY "${build_dir}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${description} failed (${result}):\n${output}\n${error}")
    endif()
endfunction()

function(test_tweakme_configuration library_type shared)
    set(build_dir "${SPDLOG_TEST_BINARY_DIR}/${library_type}/build")
    set(prefix "${SPDLOG_TEST_BINARY_DIR}/${library_type}/prefix")
    set(consumer_dir "${SPDLOG_TEST_BINARY_DIR}/${library_type}/consumer")
    file(REMOVE_RECURSE "${SPDLOG_TEST_BINARY_DIR}/${library_type}")

    set(generator_args -G "${SPDLOG_TEST_GENERATOR}")
    if(SPDLOG_TEST_GENERATOR_PLATFORM)
        list(APPEND generator_args -A "${SPDLOG_TEST_GENERATOR_PLATFORM}")
    endif()
    if(SPDLOG_TEST_GENERATOR_TOOLSET)
        list(APPEND generator_args -T "${SPDLOG_TEST_GENERATOR_TOOLSET}")
    endif()

    set(common_args
        "-DCMAKE_CXX_COMPILER=${SPDLOG_TEST_CXX_COMPILER}"
        "-DCMAKE_BUILD_TYPE=${SPDLOG_TEST_CONFIG}")

    configure_checked(
        "Configure ${library_type} source-tree consumer"
        "${CMAKE_CURRENT_LIST_DIR}/tweakme" "${build_dir}" ${generator_args} ${common_args}
        "-DSPDLOG_TEST_SOURCE_DIR=${SPDLOG_TEST_SOURCE_DIR}" "-DSPDLOG_TEST_SHARED=${shared}"
        "-DCMAKE_INSTALL_PREFIX=${prefix}")
    run_checked("Build ${library_type} source-tree consumer" "${CMAKE_COMMAND}" --build "${build_dir}" --config
                "${SPDLOG_TEST_CONFIG}")
    run_checked("Install ${library_type} spdlog" "${CMAKE_COMMAND}" --build "${build_dir}" --target install --config
                "${SPDLOG_TEST_CONFIG}")

    set(installed_tweakme "${prefix}/include/spdlog/tweakme.h")
    file(READ "${installed_tweakme}" tweakme_contents)
    foreach(expected_macro SPDLOG_NO_THREAD_ID SPDLOG_NO_TLS SPDLOG_DISABLE_DEFAULT_LOGGER)
        if(NOT tweakme_contents MATCHES "#define ${expected_macro}([\r\n]|$)")
            message(FATAL_ERROR "${installed_tweakme} does not define ${expected_macro}")
        endif()
    endforeach()

    configure_checked(
        "Configure ${library_type} installed consumers"
        "${CMAKE_CURRENT_LIST_DIR}/tweakme" "${consumer_dir}" ${generator_args} ${common_args}
        -DSPDLOG_TEST_INSTALLED=ON "-DSPDLOG_TEST_PREFIX=${prefix}" "-DCMAKE_PREFIX_PATH=${prefix}")
    run_checked("Build ${library_type} installed consumers" "${CMAKE_COMMAND}" --build "${consumer_dir}" --config
                "${SPDLOG_TEST_CONFIG}")
endfunction()

test_tweakme_configuration(static OFF)
test_tweakme_configuration(shared ON)
