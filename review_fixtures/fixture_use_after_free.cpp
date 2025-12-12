#include <cstdlib>
#include <spdlog/spdlog.h>

int main() {
    spdlog::info("fixture_use_after_free start");

    char *p = static_cast<char *>(std::malloc(8));
    if (!p) {
        return 1;
    }
    p[0] = 'A';
    std::free(p);
    p[1] = 'B';

    spdlog::info("fixture_use_after_free end {}", p[0]);
    return 0;
}
