#include "includes.h"

using namespace std::chrono;
using std::chrono::milliseconds;
using std::chrono::system_clock;

system_clock::time_point now_millis()
{
    return time_point_cast<milliseconds>(system_clock::now());
}
TEST_CASE("dequeue-empty-nowait", "[mpmc_blocking_q]")
{
    size_t q_size = 100;
    milliseconds tolerance_wait(10);
    spdlog::details::mpmc_blocking_queue<int> q(q_size);
    int popped_item;

    auto millis_0 = now_millis();
    auto rv = q.dequeue_for(popped_item, milliseconds::zero());
    auto millis_1 = now_millis();

    REQUIRE(rv == false);
    REQUIRE((millis_1 - millis_0) <= tolerance_wait);
}

TEST_CASE("dequeue-empty-wait", "[mpmc_blocking_q]")
{

    size_t q_size = 100;
    milliseconds wait_ms(250);
    milliseconds tolerance_wait(10);

    spdlog::details::mpmc_blocking_queue<int> q(q_size);
    int popped_item;
    auto millis_0 = now_millis();
    auto rv = q.dequeue_for(popped_item, wait_ms);
    auto millis_1 = now_millis();
    auto delta_ms = millis_1 - millis_0;

    REQUIRE(rv == false);
    REQUIRE(delta_ms >= wait_ms);
    REQUIRE(delta_ms <= wait_ms + tolerance_wait);
}

TEST_CASE("enqueue_nowait", "[mpmc_blocking_q]")
{

    size_t q_size = 1;
    spdlog::details::mpmc_blocking_queue<int> q(q_size);
    milliseconds tolerance_wait(10);

    q.enqueue(1);
    REQUIRE(q.overrun_counter() == 0);

    auto millis_0 = now_millis();
    q.enqueue_nowait(2);
    auto millis_1 = now_millis();
    REQUIRE((millis_1 - millis_0) <= tolerance_wait);
    REQUIRE(q.overrun_counter() == 1);
}

TEST_CASE("bad_queue", "[mpmc_blocking_q]")
{
    size_t q_size = 0;
    spdlog::details::mpmc_blocking_queue<int> q(q_size);
    q.enqueue_nowait(1);
    REQUIRE(q.overrun_counter() == 1);
    int i;
    REQUIRE(q.dequeue_for(i, milliseconds(0)) == false);
}

TEST_CASE("empty_queue", "[mpmc_blocking_q]")
{
    size_t q_size = 10;
    spdlog::details::mpmc_blocking_queue<int> q(q_size);
    int i;
    REQUIRE(q.dequeue_for(i, milliseconds(10)) == false);
}

TEST_CASE("full_queue", "[mpmc_blocking_q]")
{
    size_t q_size = 100;
    spdlog::details::mpmc_blocking_queue<int> q(q_size);
    for (int i = 0; i < static_cast<int>(q_size); i++)
    {
        q.enqueue(std::move(i));
    }

    q.enqueue_nowait(123456);
    REQUIRE(q.overrun_counter() == 1);

    for (int i = 1; i < static_cast<int>(q_size); i++)
    {
        int item = -1;
        q.dequeue_for(item, milliseconds(0));
        REQUIRE(item == i);
    }

    // last item pushed has overridden the oldest.
    int item = -1;
    q.dequeue_for(item, milliseconds(0));
    REQUIRE(item == 123456);
}