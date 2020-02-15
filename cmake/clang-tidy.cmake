option(SPDLOG_ENABLE_TIDY "Add clang-tidy automatically as prebuild step" OFF)

find_program(CLANG_TIDY_EXE
        NAMES clang-tidy clang-tidy-7
        DOC "Path to clang-tidy executable")
find_program(CLANG_TIDY_RUNNER
        NAMES run-clang-tidy run-clang-tidy-7 run-clang-tidy.py
        HINTS ${CMAKE_CURRENT_SOURCE_DIR}/scripts)
if(CLANG_TIDY_EXE)
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    set(DCMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(CLANG_TIDY_CMD ${CLANG_TIDY_EXE})
    message(STATUS "cmake source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
    # NOTE: copy default checks config file .clang-tidy for later use
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy ${CMAKE_CURRENT_BINARY_DIR} @ONLY)

    if(SPDLOG_ENABLE_TIDY)
        # NOTE: the project config file .clang-tidy is not found if the
        # binary tree is not part of the source tree! CK
        set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_CMD} CACHE STRING "" FORCE)
    else()
        set(CMAKE_CXX_CLANG_TIDY "" CACHE STRING "" FORCE) # delete it
    endif()

    if(CLANG_TIDY_RUNNER)
        if(NOT TARGET check)
            add_custom_target(check)
            message(STATUS "check target added")
            set_target_properties(check PROPERTIES EXCLUDE_FROM_ALL TRUE)
        endif()

        add_custom_command(TARGET check PRE_BUILD
            # -p BUILD_PATH Path used to read a compile command database (compile_commands.json).
            # NOTE: we use default checks from .clang-tidy
            COMMAND ${CLANG_TIDY_RUNNER} -p ${CMAKE_CURRENT_BINARY_DIR}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}   # location of compile_commands.json
            COMMENT "Running check on targets at ${CMAKE_CURRENT_SOURCE_DIR} ..."
            VERBATIM
        )
    endif()
else()
    message(AUTHOR_WARNING "clang-tidy not found!")
endif()
