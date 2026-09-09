#if defined(__GNUC__) && __GNUC__ == 12
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"  // Workaround for GCC 12
#endif

#ifdef SPDLOG_USE_CATCH2_V2
#    define CATCH_CONFIG_MAIN
#    include <catch2/catch.hpp>
#else
#    include <catch2/catch_all.hpp>
#endif

#if defined(__GNUC__) && __GNUC__ == 12
#pragma GCC diagnostic pop
#endif
