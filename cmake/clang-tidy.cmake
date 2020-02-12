option(ENABLE_CLANG_TIDY "Add run-clang-tidy automatically to builds" OFF)
find_program(CLANG_TIDY_EXE
        NAMES run-clang-tidy.py run-clang-tidy run-clang-tidy-7
        DOC "Path to clang-tidy executable")
if(CLANG_TIDY_EXE)
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    set(DCMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(CLANG_TIDY_CHECKS "'-*,cppcoreguidelines-*,-cppcoreguidelines-macro-usage'")
    set(CLANG_TIDY_CMD "${CLANG_TIDY_EXE};-checks=${CLANG_TIDY_CHECKS};-header-filter='${CMAKE_SOURCE_DIR}/*'")
    message(STATUS "cmake source dir: ${CMAKE_SOURCE_DIR}")

    if(ENABLE_CLANG_TIDY)
        set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_CMD} CACHE STRING "" FORCE)
    else()
        set(CMAKE_CXX_CLANG_TIDY "" CACHE STRING "" FORCE) # delete it
    endif()

    if(NOT TARGET check)
        add_custom_target(check)
        message(STATUS "check target added")
        set_target_properties(check PROPERTIES EXCLUDE_FROM_ALL TRUE)
    endif()

    add_custom_command(TARGET check PRE_BUILD
        # -p BUILD_PATH Path used to read a compile command database (compile_commands.json).
        # NOTE: we use defaults checks from .clang-tidy
        COMMAND ${CLANG_TIDY_EXE} -p ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"   # location of compile_commands.json
        COMMENT "Running check on targets at ${CMAKE_SOURCE_DIR} ..."
        VERBATIM
    )
else()
    message(AUTHOR_WARNING "run-clang-tidy not found!")
endif()
