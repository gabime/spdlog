#define SPDLOG_ENABLE_LOGMSG_METADATA 1

#include "includes.h"

class NullSink : public spdlog::sinks::sink
{
public:
    void log(const spdlog::details::log_msg&) override;
    void flush() override;
};

void NullSink::flush() {}

void NullSink::log(const spdlog::details::log_msg& msg)
{
    // fake sink output
    //std::cout << msg.formatted.str() << std::endl;
    {
        auto iter = msg.attrs.find("param_int");
        if (iter == msg.attrs.end())
            FAIL("log message missing 'param_int' attribute");
        REQUIRE( 42 == spdlog::attrval::get<int>( iter->second ) );
    }
    {
        auto iter = msg.attrs.find("param_float");
        if (iter == msg.attrs.end())
            FAIL("log message missing 'param_float' attribute");
        REQUIRE( 42.5 == spdlog::attrval::get<float>( iter->second ) );
    }
    {
        auto iter = msg.attrs.find("param_double");
        if (iter == msg.attrs.end())
            FAIL("log message missing 'param_double' attribute");
        REQUIRE( 42.5 == spdlog::attrval::get<double>( iter->second ) );
    }
    {
        auto iter = msg.attrs.find("param_bool");
        if (iter == msg.attrs.end())
            FAIL("log message missing 'param_bool' attribute");
        REQUIRE( spdlog::attrval::get<double>( iter->second ) );
    }
    {
        auto iter = msg.attrs.find("param_string");
        if (iter == msg.attrs.end())
            FAIL("log message missing 'param_string' attribute");
        REQUIRE("spdlog feature test" == spdlog::attrval::get<std::string>( iter->second ) );
    }
}

static std::string log_to_str(spdlog::attrmap_type& attrs, const std::string& msg)
{
	std::ostringstream oss;
	auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
	spdlog::logger oss_logger("attr_tester", oss_sink);
	oss_logger.set_level(spdlog::level::info);
    auto formatter = std::make_shared<spdlog::pattern_formatter>("%v", spdlog::pattern_time_type::local, "");
    oss_logger.set_formatter(formatter);
	oss_logger.info(attrs, msg);
	return oss.str();
}

TEST_CASE("log attr tests", "[log_attrs]")
{
    SECTION("attributes test") {
        // supported types: int, double, float, bool, std::string
        spdlog::attrmap_type attrs {
            {"param_int", 42},
            {"param_double", 42.5},
            {"param_float", 42.5},
            {"param_bool", true},
            {"param_string", "spdlog feature test"},
        };
        REQUIRE(log_to_str(attrs, "attributes enabled") == "attributes enabled");
    }
}

