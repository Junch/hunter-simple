#include <boost/type_index.hpp>
#include <functional>
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <string>

template <typename T> void printType(const T &param)
{
    using boost::typeindex::type_id_with_cvr;

    // Show T
    std::cout << "T = " << type_id_with_cvr<T>().pretty_name() << '\n';

    // // Show param's type
    // std::cout << "T = " << type_id_with_cvr<decltype(param)>().pretty_name()
    // << '\n';
}

TEST(typeindex, shared_ptr)
{
    const auto r = std::make_shared<int>(5);
    printType(r);

    using boost::typeindex::type_id_with_cvr;
    std::cout << "r = " << type_id_with_cvr<decltype(r)>().pretty_name() << '\n';
}

void output(int x, int y) { printf("x=%d, y=%d\n", x, y); }

TEST(typeindex, bind)
{
    auto fr = std::bind(output, std::placeholders::_1, 2);
    fr(3);
    printType(fr);
}

TEST(typeindex, lambda)
{
    auto f = [](int a) -> int { return a + 1; };
    printType(f);
    printf("%d\n", f(4));
}

TEST(typeindex, loop)
{
    std::map<int, std::string> m;
    m.emplace(1, "Tom");
    m.emplace(2, "Marry");

    for (auto &r : m)
    {
        printType(r);
        printf("#%d -> %s\n", r.first, r.second.c_str());
    }

    auto iter = std::find_if(cbegin(m), cend(m), [](auto &i) {
        printType(i);
        return i.second == std::string("Marry");
    });

    printType(iter);

    if (iter != cend(m))
    {
        printf("#%d -> %s\n", iter->first, iter->second.c_str());
    }
}
