# Drives the C++20 module through a full install/consume cycle:
#   1. build and install spdlog from source with SPDLOG_BUILD_MODULE=ON
#   2. assert the module targets landed in their own targets file, so that the
#      "CMake >= 3.28 required" guard never reaches the classic consumers
#   3. build a consumer that imports the module from the install, which is what
#      compiles the installed spdlog.cppm into a BMI in the consumer's build
#   4. build a CMake 3.10 / C++11 consumer against the same install

function(run_checked description)
    execute_process(COMMAND ${ARGN} RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${description} failed (${result}):
${output}
${error}")
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
        message(FATAL_ERROR "${description} failed (${result}):
${output}
${error}")
    endif()
endfunction()

function(test_module_configuration library_type shared)
    set(build_dir "${SPDLOG_TEST_BINARY_DIR}/${library_type}/build")
    set(prefix "${SPDLOG_TEST_BINARY_DIR}/${library_type}/prefix")
    set(consumer_dir "${SPDLOG_TEST_BINARY_DIR}/${library_type}/consumer")
    set(legacy_dir "${SPDLOG_TEST_BINARY_DIR}/${library_type}/legacy")
    file(REMOVE_RECURSE "${SPDLOG_TEST_BINARY_DIR}/${library_type}")

    set(generator_args -G "${SPDLOG_TEST_GENERATOR}")
    set(common_args "-DCMAKE_CXX_COMPILER=${SPDLOG_TEST_CXX_COMPILER}" "-DCMAKE_BUILD_TYPE=${SPDLOG_TEST_CONFIG}")

    # --- 1. build and install from source -------------------------------------------
    configure_checked(
        "Configure ${library_type} source-tree module consumer"
        "${CMAKE_CURRENT_LIST_DIR}/module" "${build_dir}" ${generator_args} ${common_args}
        "-DSPDLOG_TEST_SOURCE_DIR=${SPDLOG_TEST_SOURCE_DIR}" "-DSPDLOG_TEST_SHARED=${shared}"
        "-DCMAKE_INSTALL_PREFIX=${prefix}")
    run_checked("Build ${library_type} source-tree module consumer" "${CMAKE_COMMAND}" --build "${build_dir}"
                --config "${SPDLOG_TEST_CONFIG}")
    run_checked("Install ${library_type} spdlog" "${CMAKE_COMMAND}" --build "${build_dir}" --target install
                --config "${SPDLOG_TEST_CONFIG}")

    # --- 2. the export sets must be separate ----------------------------------------
    set(classic_targets "${prefix}/lib/cmake/spdlog/spdlogConfigTargets.cmake")
    set(module_targets "${prefix}/lib/cmake/spdlog/spdlogModuleTargets.cmake")
    set(module_source "${prefix}/share/spdlog/module/spdlog.cppm")

    if(NOT EXISTS "${module_targets}")
        message(FATAL_ERROR "${module_targets} was not installed")
    endif()
    if(NOT EXISTS "${module_source}")
        message(FATAL_ERROR "${module_source} was not installed")
    endif()

    file(READ "${classic_targets}" classic_contents)
    if(classic_contents MATCHES "CMake >= 3[.]2[89]")
        message(FATAL_ERROR
                "${classic_targets} carries a CMake >= 3.28 requirement, which would break existing consumers")
    endif()

    file(READ "${module_targets}" module_contents)
    if(NOT module_contents MATCHES "3[.]28")
        message(FATAL_ERROR "${module_targets} does not look like it exports a CXX_MODULES file set")
    endif()

    # --- 3. consume the installed module ---------------------------------------------
    configure_checked(
        "Configure ${library_type} installed module consumer"
        "${CMAKE_CURRENT_LIST_DIR}/module" "${consumer_dir}" ${generator_args} ${common_args}
        -DSPDLOG_TEST_INSTALLED=ON "-DCMAKE_PREFIX_PATH=${prefix}")
    run_checked("Build ${library_type} installed module consumer" "${CMAKE_COMMAND}" --build "${consumer_dir}"
                --config "${SPDLOG_TEST_CONFIG}")

    # --- 4. an old CMake / C++11 consumer of the same install -------------------------
    configure_checked(
        "Configure ${library_type} legacy consumer"
        "${CMAKE_CURRENT_LIST_DIR}/module/legacy" "${legacy_dir}" ${generator_args} ${common_args}
        "-DCMAKE_PREFIX_PATH=${prefix}" -DCMAKE_POLICY_VERSION_MINIMUM=3.10)
    run_checked("Build ${library_type} legacy consumer" "${CMAKE_COMMAND}" --build "${legacy_dir}"
                --config "${SPDLOG_TEST_CONFIG}")
endfunction()

test_module_configuration(static OFF)
test_module_configuration(shared ON)
