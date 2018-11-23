#include "predicate.h"
#include "gtest/gtest.h"
#include <atomic>
#include <thread>
using namespace std::chrono_literals;

std::atomic<bool> ready{false};
TEST(predicate, not_timeout)
{
    ready = false;

    std::thread thd([] {
        std::this_thread::sleep_for(300ms);
        ready = true;
    });

    predicate p([] { return ready.load(); }, 400);
    ASSERT_TRUE(p.evaluate());

    thd.join();
}

TEST(predicate, timeout)
{
    ready = false;

    std::thread thd([] {
        std::this_thread::sleep_for(500ms);
        ready = true;
    });

    predicate p([] { return ready.load(); }, 400);
    ASSERT_FALSE(p.evaluate());

    thd.join();
}

TEST(predicate, timeout_2)
{
    predicate p([] { return false; }, 1000);
    ASSERT_FALSE(p.evaluate());
}
