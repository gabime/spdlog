#include <cstring>
#include <spdlog/spdlog.h>

int main(int argc, char **argv) {
    spdlog::info("fixture_format_string_injection start");

    const char *user = (argc > 1) ? argv[1] : "%x %x %x %x";
    spdlog::info(user);

    spdlog::info("fixture_format_string_injection end");
    return 0;
}
