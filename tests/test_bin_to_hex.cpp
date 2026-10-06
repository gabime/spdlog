#include "includes.h"
#include "test_sink.h"
#include "spdlog/fmt/bin_to_hex.h"
#include <limits>

TEST_CASE("to_hex_large_line_width", "[to_hex]") {
    const std::vector<unsigned char> bytes{'A', 'B', 0, 255};
    const size_t width = GENERATE(size_t(32), size_t((std::numeric_limits<int>::max)()) + 1,
                                  (std::numeric_limits<size_t>::max)());
    const bool use_range = GENERATE(false, true);
    const bool show_ascii = GENERATE(false, true);
    std::ostringstream oss;
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    sink->set_pattern("%v");
    spdlog::logger logger("hex", sink);
    if (use_range) {
        if (show_ascii) {
            logger.info("{:a}", spdlog::to_hex(bytes.begin(), bytes.end(), width));
        } else {
            logger.info("{}", spdlog::to_hex(bytes.begin(), bytes.end(), width));
        }
    } else {
        if (show_ascii) {
            logger.info("{:a}", spdlog::to_hex(bytes, width));
        } else {
            logger.info("{}", spdlog::to_hex(bytes, width));
        }
    }
    const std::string eol = spdlog::details::os::default_eol;
    REQUIRE(oss.str() == eol + "0000: 41 42 00 ff" + (show_ascii ? "  AB.." : "") + eol);
}

TEST_CASE("to_hex_zero_line_width", "[to_hex]") {
    const std::vector<unsigned char> bytes{'A', 'B'};
    std::ostringstream oss;
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    sink->set_pattern("%v");
    spdlog::logger logger("hex", sink);
    logger.info("{:a}", spdlog::to_hex(bytes, 0));
    const std::string eol = spdlog::details::os::default_eol;
    REQUIRE(oss.str() == eol + "0000: 41  A" + eol + "0001: 42  B" + eol);
}

TEST_CASE("to_hex", "[to_hex]") {
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    spdlog::logger oss_logger("oss", oss_sink);

    std::vector<unsigned char> v{9, 0xa, 0xb, 0xc, 0xff, 0xff};
    oss_logger.info("{}", spdlog::to_hex(v));

    auto output = oss.str();
    REQUIRE(ends_with(output,
                      "0000: 09 0a 0b 0c ff ff" + std::string(spdlog::details::os::default_eol)));
}

TEST_CASE("to_hex_upper", "[to_hex]") {
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    spdlog::logger oss_logger("oss", oss_sink);

    std::vector<unsigned char> v{9, 0xa, 0xb, 0xc, 0xff, 0xff};
    oss_logger.info("{:X}", spdlog::to_hex(v));

    auto output = oss.str();
    REQUIRE(ends_with(output,
                      "0000: 09 0A 0B 0C FF FF" + std::string(spdlog::details::os::default_eol)));
}

TEST_CASE("to_hex_no_delimiter", "[to_hex]") {
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    spdlog::logger oss_logger("oss", oss_sink);

    std::vector<unsigned char> v{9, 0xa, 0xb, 0xc, 0xff, 0xff};
    oss_logger.info("{:sX}", spdlog::to_hex(v));

    auto output = oss.str();
    REQUIRE(
        ends_with(output, "0000: 090A0B0CFFFF" + std::string(spdlog::details::os::default_eol)));
}

TEST_CASE("to_hex_show_ascii", "[to_hex]") {
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    spdlog::logger oss_logger("oss", oss_sink);

    std::vector<unsigned char> v{9, 0xa, 0xb, 0x41, 0xc, 0x4b, 0xff, 0xff};
    oss_logger.info("{:Xsa}", spdlog::to_hex(v, 8));

    REQUIRE(ends_with(oss.str(), "0000: 090A0B410C4BFFFF  ...A.K.." +
                                     std::string(spdlog::details::os::default_eol)));
}

TEST_CASE("to_hex_different_size_per_line", "[to_hex]") {
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    spdlog::logger oss_logger("oss", oss_sink);

    std::vector<unsigned char> v{9, 0xa, 0xb, 0x41, 0xc, 0x4b, 0xff, 0xff};

    oss_logger.info("{:Xsa}", spdlog::to_hex(v, 10));
    REQUIRE(ends_with(oss.str(), "0000: 090A0B410C4BFFFF  ...A.K.." +
                                     std::string(spdlog::details::os::default_eol)));

    oss_logger.info("{:Xs}", spdlog::to_hex(v, 10));
    REQUIRE(ends_with(oss.str(),
                      "0000: 090A0B410C4BFFFF" + std::string(spdlog::details::os::default_eol)));

    oss_logger.info("{:Xsa}", spdlog::to_hex(v, 6));
    REQUIRE(ends_with(
        oss.str(), "0000: 090A0B410C4B  ...A.K" + std::string(spdlog::details::os::default_eol) +
                       "0006: FFFF          .." + std::string(spdlog::details::os::default_eol)));

    oss_logger.info("{:Xs}", spdlog::to_hex(v, 6));
    REQUIRE(ends_with(oss.str(), "0000: 090A0B410C4B" +
                                     std::string(spdlog::details::os::default_eol) + "0006: FFFF" +
                                     std::string(spdlog::details::os::default_eol)));
}

TEST_CASE("to_hex_no_ascii", "[to_hex]") {
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    spdlog::logger oss_logger("oss", oss_sink);

    std::vector<unsigned char> v{9, 0xa, 0xb, 0x41, 0xc, 0x4b, 0xff, 0xff};
    oss_logger.info("{:Xs}", spdlog::to_hex(v, 8));

    REQUIRE(ends_with(oss.str(),
                      "0000: 090A0B410C4BFFFF" + std::string(spdlog::details::os::default_eol)));

    oss_logger.info("{:Xsna}", spdlog::to_hex(v, 8));

    REQUIRE(
        ends_with(oss.str(), "090A0B410C4BFFFF" + std::string(spdlog::details::os::default_eol)));
}
