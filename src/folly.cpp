#include <atomic>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/futures/Future.h>
#include <gtest/gtest.h>
#include <iostream>

void foo(int x) { std::cout << "fool(" << x << ")\n"; }

TEST(folly, future)
{
    folly::Promise<int> p;
    folly::Future<folly::Unit> fu = p.getFuture().unit();
    EXPECT_FALSE(fu.isReady());
    p.setValue(42);
    EXPECT_TRUE(fu.isReady());
}

static folly::Func burnMs(uint64_t ms)
{
    return [ms]() { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); };
}

TEST(folly, threadpool)
{
    // https://github.com/facebook/folly/blob/master/folly/executors/test/ThreadPoolExecutorTest.cpp

    folly::CPUThreadPoolExecutor exe(10);

    std::atomic<int> sum(0);
    auto f = [&]() {
        burnMs(1)();
        sum++;
    };

    for (int i = 0; i < 100; i++)
    {
        exe.add(f);
    }

    exe.join();

    ASSERT_EQ(100, sum);
}
