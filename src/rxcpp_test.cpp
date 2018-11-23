#include <array>
#include <gtest/gtest.h>
#include <rxcpp/rx.hpp>

///////////////////////////////////////////////////////////////////////////////
// Title: Streaming values from C++ containers
// Book: C++ Reactive Programming
///////////////////////////////////////////////////////////////////////////////
TEST(rxcpp, container)
{
    std::array<int, 3> a{1, 2, 3};
    auto values = rxcpp::observable<>::iterate(a);
    values.subscribe([](int v) { printf("OnNext: %d\n", v); }, []() { printf("OnCompleted\n"); });
}

///////////////////////////////////////////////////////////////////////////////
// Title: Creating Observables from scratch
// Book: C++ Reactive Programming
///////////////////////////////////////////////////////////////////////////////
TEST(rxcpp, scratch)
{
    auto ints = rxcpp::observable<>::create<int>([](rxcpp::subscriber<int> s) {
        s.on_next(1);
        s.on_next(4);
        s.on_next(9);
        s.on_completed();
    });

    ints.subscribe([](int v) { printf("OnNext: %d\n", v); }, []() { printf("OnCompleted\n"); });
}

///////////////////////////////////////////////////////////////////////////////
// Title: Concatenating Observerable Streams
// Book: C++ Reactive Programming
///////////////////////////////////////////////////////////////////////////////
TEST(rxcpp, concat)
{
    auto values = rxcpp::observable<>::range(1);
    auto s1 = values.take(3).map([](int prime) { return 2 * prime; });
    auto s2 = values.take(3).map([](int prime) { return prime * prime; });
    s1.concat(s2).subscribe([](int i) { printf("OnNext: %d\n", i); }, []() { printf("OnCompleted\n"); });
}
