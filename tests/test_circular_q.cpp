#include "includes.h"
#include "spdlog/details/circular_q.h"

using q_type = spdlog::details::circular_q<size_t>;
TEST_CASE("test_size", "[circular_q]")
{
    const size_t q_size = 4;
    q_type q(q_size);
    REQUIRE(q.size() == 0);
    REQUIRE(q.empty() == true);
    for (size_t i = 0; i < q_size; i++)
    {
        q.push_back(std::move(i));
    }
    REQUIRE(q.size() == q_size);
    q.push_back(999);
    REQUIRE(q.size() == q_size);
}

TEST_CASE("test_rolling", "[circular_q]")
{
    const size_t q_size = 4;
    q_type q(q_size);

    for (size_t i = 0; i < q_size + 2; i++)
    {
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

TEST_CASE("test_empty", "[circular_q]")
{
    q_type q(0);
    q.push_back(1);
    REQUIRE(q.empty());
}