include(FetchContent)

# Always build static fmtlib version
set(BUILD_SHARED_LIBS_ORIG "${BUILD_SHARED_LIBS}")
#set(BUILD_SHARED_LIBS OFF CACHE BOOL "Disable BUILD_SHARED_LIBS for fmtlib" FORCE)

Set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG f5e54359df4c26b6230fc61d38aa294581393084 # 10.1.1
        GIT_PROGRESS TRUE
)

FetchContent_GetProperties(fmt)
if(NOT fmt_POPULATED)
    FetchContent_Populate(fmt)
    # We do not require os features of fmt
    set(FMT_OS OFF CACHE BOOL "Disable FMT_OS" FORCE)
    add_subdirectory(${fmt_SOURCE_DIR} ${fmt_BINARY_DIR})
endif ()

# Restore BUILD_SHARED_LIBS
set(BUILD_SHARED_LIBS "${BUILD_SHARED_LIBS_ORIG}" CACHE BOOL "Restore BUILD_SHARED_LIBS" FORCE)