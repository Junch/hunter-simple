#include <gtest/gtest.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <unordered_map>

using json = nlohmann::json;

TEST(json, simple)
{
    // https://nlohmann.github.io/json/classnlohmann_1_1basic__json_a16f9445f7629f634221a42b967cdcd43.html

    // clang-format off
    // create a JSON value with different types
    json json_types =
    {
        {"boolean", true},
        {
            "number", {
                {"integer", 42},
                {"floating-point", 17.23}
            }
        },
        {"string", "Hello, world!"},
        {"array", {1, 2, 3, 4, 5}},
        {"null", nullptr}
    };
    // clang-format on

    // use explicit conversions
    auto v1 = json_types["boolean"].get<bool>();
    auto v2 = json_types["number"]["integer"].get<int>();
    auto v3 = json_types["number"]["integer"].get<short>();
    auto v4 = json_types["number"]["floating-point"].get<float>();
    auto v5 = json_types["number"]["floating-point"].get<int>();
    auto v6 = json_types["string"].get<std::string>();
    auto v7 = json_types["array"].get<std::vector<short>>();
    auto v8 = json_types.get<std::unordered_map<std::string, json>>();

    // print the conversion results
    std::cout << v1 << '\n';
    std::cout << v2 << ' ' << v3 << '\n';
    std::cout << v4 << ' ' << v5 << '\n';
    std::cout << v6 << '\n';

    for (auto i : v7)
    {
        std::cout << i << ' ';
    }
    std::cout << "\n\n";

    for (auto i : v8)
    {
        std::cout << i.first << ": " << i.second << '\n';
    }
}

TEST(json, exception)
{
    // https://nlohmann.github.io/json/classnlohmann_1_1basic__json_a28f7c2f087274a0012eb7a2333ee1580.html#a28f7c2f087274a0012eb7a2333ee1580

    try
    {
        // calling at() for an invalid index
        json j = {1, 2, 3, 4};
        j.at(4) = 10;
    }
    catch (json::out_of_range &e)
    {
        // output exception information
        std::cout << "message: " << e.what() << '\n' << "exception id: " << e.id << std::endl;
    }
}
