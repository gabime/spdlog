include(FetchContent)

FetchContent_Declare(
        fmt
        DOWNLOAD_EXTRACT_TIMESTAMP FALSE
        URL https://github.com/fmtlib/fmt/archive/refs/tags/10.2.1.tar.gz
        URL_HASH SHA256=1250e4cc58bf06ee631567523f48848dc4596133e163f02615c97f78bab6c811)
FetchContent_GetProperties(fmt)
if(NOT fmt_POPULATED)
    FetchContent_Populate(fmt)
    # We do not require os features of fmt
    set(FMT_OS OFF CACHE BOOL "Disable FMT_OS" FORCE)
    add_subdirectory(${fmt_SOURCE_DIR} ${fmt_BINARY_DIR})
	set_target_properties(fmt PROPERTIES FOLDER "third-party")
endif ()
