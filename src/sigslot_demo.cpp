#include <gtest/gtest.h>
#include <iostream>
#include <sigslot/signal.hpp>

namespace sigslot_demo
{

void f()
{
    std::cout << "free function\n";
}

struct s
{
    void m()
    {
        std::cout << "member function\n";
    }
    static void sm()
    {
        std::cout << "static member function\n";
    }
};

struct o
{
    void operator()()
    {
        std::cout << "function object\n";
    }
};

TEST(sigslot, no_arguments)
{
    s d;
    auto lambda = []() { std::cout << "lambda\n"; };
    auto gen_lambda = [](auto &&... a) { std::cout << "generic lambda\n"; };

    // declare a signal instance with no arguments
    sigslot::signal<> sig;

    // connect slots
    sig.connect(f);
    sig.connect(&s::m, &d);
    sig.connect(&s::sm);
    sig.connect(o());
    sig.connect(lambda);
    sig.connect(gen_lambda);

    // emit a signal
    sig();
}

struct foo
{
    // Notice how we accept a double as first argument here.
    // This is fine because float is convertible to double.
    // 's' is a reference and can thus be modified.
    void bar(double d, int i, bool b, std::string &s)
    {
        s = b ? std::to_string(i) : std::to_string(d);
    }
};

// Function objects can cope with default arguments and overloading.
// It does not work with static and member functions.
struct obj
{
    void operator()(float, int, bool, std::string &, int = 0)
    {
        std::cout << "I was here\n";
    }
};

TEST(sigslot, arguments)
{
    // declare a signal with float, int, bool and string& arguments
    sigslot::signal<float, int, bool, std::string &> sig;

    // a generic lambda that prints its arguments to stdout
    auto printer = [](auto a, auto &&... args) {
        std::cout << a;
        (void)std::initializer_list<int>{((void)(std::cout << " " << args), 1)...};
        std::cout << "\n";
    };

    // connect the slots
    foo ff;
    sig.connect(printer);
    sig.connect(&foo::bar, &ff);
    sig.connect(obj());

    float f = 1.f;
    short i = 2; // convertible to int
    std::string s = "0";

    // emit a signal
    sig(f, i, false, s);
    sig(f, i, true, s);
}

} // namespace sigslot_demo