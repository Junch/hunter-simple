#include <boost/type_index.hpp>
#include <gtest/gtest.h>
using boost::typeindex::type_id_with_cvr;

void print(int &t) { std::cout << "lvalue" << '\n'; }

void print(int &&t) { std::cout << "rvalue" << '\n'; }

template <typename T> void testForward(T &&v)
{
    std::cout << "type: " << type_id_with_cvr<decltype(v)>().pretty_name() << '\n';
    std::cout << "typeid: " << typeid(decltype(v)).name() << '\n';
    print(v);                  // always call the lvalue version
    print(std::forward<T>(v)); // Call the version according to the type of v
    print(std::move(v));       // always call the rvalue version
}

TEST(rvalue, forward)
{
    testForward(1);
    int x = 1;
    testForward(x);
}