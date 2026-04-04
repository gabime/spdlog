include(FetchContent)

FetchContent_Declare(
    fmt
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
    URL https://github.com/fmtlib/fmt/archive/refs/tags/12.1.0.tar.gz
    URL_HASH SHA256=ea7de4299689e12b6dddd392f9896f08fb0777ac7168897a244a6d6085043fea)

FetchContent_GetProperties(fmt)
if(NOT fmt_POPULATED)
    # We do not require os features of fmt
    set(FMT_OS OFF CACHE BOOL "Disable FMT_OS" FORCE)
    # fmt 12+ defaults FMT_INSTALL to OFF when built as a subproject; spdlog's
    # install(EXPORT) requires fmt to participate in an export set (CMake 3.23+).
    set(FMT_INSTALL ON CACHE BOOL "Generate the install target." FORCE)
    FetchContent_MakeAvailable(fmt)
    set_target_properties(fmt PROPERTIES FOLDER "third-party")
    # fmt 12.1.0: MSVC C4834 on locale_ref ctor (discarded [[nodiscard]] from isalpha); fixed on fmt master after 12.1.0.
    if (MSVC)
        target_compile_options(fmt PRIVATE /wd4834)
    endif ()
endif()
