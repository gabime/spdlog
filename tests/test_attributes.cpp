#include <future>

#include "includes.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/spdlog.h"
#include "test_sink.h"

using namespace spdlog;

std::string format_attrs(std::string_view const log_name, log_attributes::attr_map_t const& attrs = {}) {
    std::string fmt_attrs;
    for (auto&& attr : attrs) fmt_attrs += fmt_lib::format("{}:{} ", attr.first, attr.second);
    if (!fmt_attrs.empty()) fmt_attrs.pop_back();

    return fmt_lib::format("[{}] [{}]", log_name, fmt_attrs);
}

std::pair<logger, std::shared_ptr<sinks::test_sink_st>> make_logger() {
    auto test_sink = std::make_shared<sinks::test_sink_st>();
    auto log = logger("logger", test_sink);
    log.set_pattern("[%n] [%*]");
    return std::make_pair(log, test_sink);
}

TEST_CASE("attribute test - multiple (single thread)") {
    auto test_sink = std::make_shared<sinks::test_sink_st>();
    logger log_a("log_a", test_sink);
    logger log_b("log_b", test_sink);
    log_a.set_pattern("[%n] [%*]");
    log_b.set_pattern("[%n] [%*]");

    log_a.attrs().put("my_key", "my_value");

    log_a.info("Hello");
    log_b.info("Hello");

    auto expected_log_a = format_attrs("log_a", {{"my_key", "my_value"}});
    auto expected_log_b = format_attrs("log_b");

    auto [found, value] = log_a.attrs().get("my_key");
    REQUIRE(found);
    REQUIRE(value->first == "my_key");
    REQUIRE(value->second == "my_value");

    auto [not_found, _] = log_a.attrs().get("my_non_existent_key");
    REQUIRE(!not_found);

    auto lines = test_sink->lines();
    REQUIRE(lines.size() == 2);
    REQUIRE(lines[0] == expected_log_a);
    REQUIRE(lines[1] == expected_log_b);
}

TEST_CASE("attribute test - remove") {
    std::vector<log_attributes::attr_map_t::value_type> attrs{
        {"my_key", "my_value"},
        {"my_key2", "my_value2"},
    };

    auto [log, test_sink] = make_logger();

    log.attrs().put(attrs[0].first, attrs[0].second);
    log.attrs().put(attrs[1].first, attrs[1].second);

    log.info("");
    auto expected_log = format_attrs("logger", {attrs.begin(), attrs.end()});

    REQUIRE(!test_sink->lines().empty());
    REQUIRE(test_sink->lines().back() == expected_log);

    log.attrs().remove(attrs.front().first);
    log.info("");
    expected_log = format_attrs("logger", {attrs.begin() + 1, attrs.end()});
    REQUIRE(test_sink->lines().back() == expected_log);

    log.attrs().remove(attrs.back().first);
    log.info("");
    expected_log = format_attrs("logger");
    REQUIRE(test_sink->lines().back() == expected_log);
}

TEST_CASE("attribute test - from range") {
    std::vector<log_attributes::attr_map_t::value_type> attrs{
        {"my_key", "my_value"},
        {"my_key2", "my_value2"},
    };

    auto [log, test_sink] = make_logger();

    log.attrs().put({attrs.begin(), attrs.end()});
    log.info("");
    auto expected_log = format_attrs("logger", {attrs.begin(), attrs.end()});

    REQUIRE(!test_sink->lines().empty());
    REQUIRE(test_sink->lines().back() == expected_log);
}

TEST_CASE("attribute test - scoped") {
    std::vector<log_attributes::attr_map_t::value_type> attrs{
        {"my_key", "my_value"},
        {"my_key2", "my_value2"},
    };

    auto [log, test_sink] = make_logger();

    auto key_value = std::make_pair("additional_key", "additional_value");
    {
        // add the attributes in scope
        auto ctx = log.attrs().scoped_ctx({attrs.begin(), attrs.end()});
        log.info("");
        auto expected_log = format_attrs("logger", {attrs.begin(), attrs.end()});

        REQUIRE(!test_sink->lines().empty());
        REQUIRE(test_sink->lines().back() == expected_log);

        // remove one of the scoped attributes
        log.attrs().remove(attrs.front().first);
        log.info("");
        expected_log = format_attrs("logger", {attrs.begin() + 1, attrs.end()});
        REQUIRE(test_sink->lines().back() == expected_log);

        // add another non-scoped attribute
        log.attrs().put(key_value.first, key_value.second);
        log.info("");
        expected_log = format_attrs("logger", {attrs[1], key_value});
        REQUIRE(test_sink->lines().back() == expected_log);
    }

    log.info("");
    auto expected_log = format_attrs("logger", {key_value});
}

TEST_CASE("attribute test - nested scoped") {
    std::vector<log_attributes::attr_map_t::value_type> attrs{
        {"my_key", "my_value"},
        {"my_key2", "my_value2"},
    };
    std::vector<log_attributes::attr_map_t::value_type> nested_attrs{
        {"nested_key", "nested_value"},
        {"nested_key2", "nested_value2"},
    };

    auto [log, test_sink] = make_logger();

    {
        // add the attributes in scope
        auto ctx = log.attrs().scoped_ctx({attrs.begin(), attrs.end()});
        log.info("");
        auto expected_log = format_attrs("logger", {attrs.begin(), attrs.end()});

        REQUIRE(!test_sink->lines().empty());
        REQUIRE(test_sink->lines().back() == expected_log);

        // add another scoped context
        {
            auto ctx_nested = log.attrs().scoped_ctx({nested_attrs.begin(), nested_attrs.end()});
            log.info("");
            log_attributes::attr_map_t all_attrs{attrs.begin(), attrs.end()};
            all_attrs.insert(nested_attrs.begin(), nested_attrs.end());
            expected_log = format_attrs("logger", {all_attrs.begin(), all_attrs.end()});
            REQUIRE(test_sink->lines().back() == expected_log);
        }

        // delete nested scope
        log.info("");
        expected_log = format_attrs("logger", {attrs.begin(), attrs.end()});
        REQUIRE(test_sink->lines().back() == expected_log);
    }

    log.info("");
    auto expected_log = format_attrs("logger");
}

TEST_CASE("attribute test - multi threaded") {
    const unsigned int n_tasks = std::thread::hardware_concurrency();
    constexpr auto n_values = 30;
    auto mt_sink = std::make_shared<spdlog::sinks::test_sink_mt>();
    auto logger = spdlog::logger("logger", mt_sink);
    logger.set_pattern("[%n] [%*]");

    // put attributes with multiple threads simultaneously
    std::vector<std::future<void>> tasks;
    for (unsigned int i = 0; i < n_tasks; ++i) {
        auto task = std::async([&logger, i, n_values] {
            for (auto j = 0; j < n_values; ++j)
                logger.attrs().put(fmt_lib::format("log_{}_key_{}", i, j), fmt_lib::format("log_{}_value_{}", i, j));
        });
        tasks.emplace_back(std::move(task));
    }

    for (auto&& task : tasks) task.wait();

    logger.info("");
    REQUIRE(!mt_sink->lines().empty());
    auto log_line = mt_sink->lines().back();
    for (unsigned int i = 0; i < n_tasks; ++i) {
        for (auto j = 0; j < n_values; ++j) {
            auto search_term = fmt_lib::format("log_{0}_key_{1}:log_{0}_value_{1}", i, j);
            REQUIRE(log_line.find(search_term) != std::string::npos);
        }
    }
}