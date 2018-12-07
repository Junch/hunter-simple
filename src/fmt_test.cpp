#include <fmt/core.h>
#include <gtest/gtest.h>

TEST(fmt, print)
{
    fmt::print("The answer is {}.\n", 42);
}
