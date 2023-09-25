#if defined(__GNUC__) && __GNUC__ == 12
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"  // Workaround for GCC 12
#endif

#include <catch2/catch_all.hpp>

#if defined(__GNUC__) && __GNUC__ == 12
    #pragma GCC diagnostic pop
#endif
