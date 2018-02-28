# Locate clang-format tool
# This module defines
#  CLANG_FORMAT_BIN
#  CLANG_FORMAT_VERSION_STR
#  CLANG_FORMAT_VERSION

find_program(
    CLANG_FORMAT_BIN
    NAMES
    clang-format-5.0
    clang-format
    DOC "clang-format binary"
)

mark_as_advanced(CLANG_FORMAT_BIN)

if(CLANG_FORMAT_BIN)
    execute_process(
        COMMAND ${CLANG_FORMAT_BIN} --version
        OUTPUT_VARIABLE CLANG_FORMAT_VERSION_STR
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    string(
        REGEX REPLACE
        "clang-format version ([.0-9]+).*" "\\1"
        CLANG_FORMAT_VERSION
        "${CLANG_FORMAT_VERSION_STR}"
    )
endif()
