#include <array>
#include <gtest/gtest.h>
#include <rxcpp/rx.hpp>

TEST(rxcpp, simple)
{
    std::array<int, 3> a{1, 2, 3};
    auto values = rxcpp::observable<>::iterate(a);
    values.subscribe([](int v) { printf("OnNext: %d\n", v); }, []() { printf("OnCompleted\n"); });
}
