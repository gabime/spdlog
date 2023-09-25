#include "includes.h"
#include "spdlog/details/circular_q.h"

using q_type = spdlog::details::circular_q<size_t>;

TEST_CASE("test_size", "[circular_q]") {
    const size_t q_size = 4;
    q_type q(q_size);
    REQUIRE(q.size() == 0);
    REQUIRE(q.empty() == true);
    REQUIRE(q.full() == false);
    for (size_t i = 0; i < q_size; i++) {
        q.push_back(10);
    }
    REQUIRE(q.size() == q_size);
    q.push_back(999);
    REQUIRE(q.size() == q_size);
}

TEST_CASE("test_rolling", "[circular_q]") {
    const size_t q_size = 4;
    q_type q(q_size);

    for (size_t i = 0; i < q_size + 2; i++) {
        auto val = i;
        q.push_back(std::move(val));  // arg to push_back must be r value
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
    q_type q1(0);
    REQUIRE(q1.empty());
    REQUIRE(q1.full());  // q with capacity 0 is considered full
    q1.push_back(1);
    REQUIRE(q1.empty());

    q_type q2(1);
    REQUIRE(q2.empty());
    q2.push_back(1);
    REQUIRE(!q2.empty());
    q2.pop_front();
    REQUIRE(q2.empty());
}

TEST_CASE("test_full", "[circular_q]") {
    q_type q1(0);
    REQUIRE(q1.full());

    q_type q2(2);
    REQUIRE(!q2.full());

    q2.push_back(1);
    REQUIRE(!q2.full());

    q2.push_back(2);
    REQUIRE(q2.full());
}

TEST_CASE("test_operator[]", "[circular_q]") {
    q_type q(2);
    q.push_back(100);
    q.push_back(200);
    REQUIRE(q[0] == 100);
    REQUIRE(q[1] == 200);
}

TEST_CASE("test_operator=", "[circular_q]") {
    q_type q1(2);
    q1.push_back(100);
    q1.push_back(200);
    q_type q2 = q1;
    REQUIRE(q2.size() == 2);
    REQUIRE(q2[0] == 100);
    REQUIRE(q2[1] == 200);
}

TEST_CASE("test_front", "[circular_q]") {
    q_type q(2);
    q.push_back(100);
    q.push_back(200);
    REQUIRE(q.front() == 100);
}

TEST_CASE("test_overrun_counter", "[circular_q]") {
    q_type q(2);
    REQUIRE(q.overrun_counter() == 0);
    for (size_t i = 0; i < 10; i++) {
        q.push_back(100);
    }
    REQUIRE(q.overrun_counter() == 8);

    q.reset_overrun_counter();
    REQUIRE(q.overrun_counter() == 0);
}

TEST_CASE("test_move", "[circular_q]") {
    q_type q1(2);
    q1.push_back(100);
    q1.push_back(200);
    q1.push_back(300);

    q_type q2 = std::move(q1);
    REQUIRE(q2.size() == 2);
    REQUIRE(q2[0] == 200);
    REQUIRE(q2[1] == 300);
    REQUIRE(q2.overrun_counter() == 1);

    REQUIRE(q1.empty());
    REQUIRE(q1.overrun_counter() == 0);
}
