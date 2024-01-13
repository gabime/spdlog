include(FetchContent)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG e69e5f977d458f2650bb346dadf2ad30c5320281 # 10.2.1
        GIT_SHALLOW ON
        GIT_PROGRESS TRUE
)

FetchContent_GetProperties(fmt)
if(NOT fmt_POPULATED)
    FetchContent_Populate(fmt)
    # We do not require os features of fmt
    set(FMT_OS OFF CACHE BOOL "Disable FMT_OS" FORCE)
    add_subdirectory(${fmt_SOURCE_DIR} ${fmt_BINARY_DIR})
	set_target_properties(fmt PROPERTIES FOLDER "third-party")
endif ()
