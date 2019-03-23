#include "spdlite.h"

int main()
{
    using namespace spdlog;
    auto l = spdlog::create_lite();
    l.set_level(spdlog::lite::level::trace);
    lite::default_logger().set_level(l.get_level());
    lite::trace("hello");
    lite::trace("hello {}", std::string("again"));
}