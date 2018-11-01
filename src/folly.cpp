#include <folly/futures/Future.h>
#include <gtest/gtest.h>
#include <iostream>

void foo(int x) { std::cout << "fool(" << x << ")\n"; }

TEST(folly, simple)
{
    folly::Promise<int> p;
    folly::Future<folly::Unit> fu = p.getFuture().unit();
    EXPECT_FALSE(fu.isReady());
    p.setValue(42);
    EXPECT_TRUE(fu.isReady());
}
