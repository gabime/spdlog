#include "includes.h"
#include "spdlog/details/circular_q.h"

using q_type = spdlog::details::circular_q<size_t>;
TEST_CASE("test_size", "[circular_q]") {
    const size_t q_size = 4;
    q_type q(q_size);
    REQUIRE(q.size() == 0);
    REQUIRE(q.empty() == true);
    for (size_t i = 0; i < q_size; i++) {
        q.push_back(std::move(i));
    }
    REQUIRE(q.size() == q_size);
    q.push_back(999);
    REQUIRE(q.size() == q_size);
}

TEST_CASE("test_rolling", "[circular_q]") {
    const size_t q_size = 4;
    q_type q(q_size);

    for (size_t i = 0; i < q_size + 2; i++) {
        q.push_back(std::move(i));
    }

    REQUIRE(q.size() == q_size);

    REQUIRE(q.front() == 2);
    q.pop_front();

    REQUIRE(q.front() == 3);
    q.pop_front();

    REQUIRE(q.front() == 4);
    q.pop_front();

    REQUIRE(q.front() == 5);
    q.pop_front();

    REQUIRE(q.empty());

    q.push_back(6);
    REQUIRE(q.front() == 6);
}

TEST_CASE("test_empty", "[circular_q]") {
    q_type q(0);
    q.push_back(1);
    REQUIRE(q.empty());
}

TEST_CASE("test_self_assignment", "[circular_q]") {
    q_type q(4);
    q.push_back(10);
    q.push_back(20);

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#pragma clang diagnostic ignored "-Wself-move"
#elif defined(__GNUC__) && __GNUC__ >= 13
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"
#endif

    q = q;
    REQUIRE(q.size() == 2);
    REQUIRE(q.front() == 10);

    q = std::move(q);
    REQUIRE(q.size() == 2);
    REQUIRE(q.front() == 10);

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__) && __GNUC__ >= 13
#pragma GCC diagnostic pop
#endif
}
