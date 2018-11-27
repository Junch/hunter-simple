#include "rang.hpp"
#include "yaml-cpp/yaml.h"
#include "gtest/gtest.h"

TEST(yaml, simple)
{
    YAML::Emitter out;
    out << "Hello, World!";
    ASSERT_STREQ("Hello, World!", out.c_str());
}

struct Vec3
{
    float x, y, z;
};

struct Power
{
    std::string name;
    int damage;
};

struct Monster
{
    std::string name;
    Vec3 position;
    std::vector<Power> powers;
};

namespace YAML
{
template <> struct convert<Vec3>
{
    static Node encode(const Vec3 &rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        return node;
    }

    static bool decode(const Node &node, Vec3 &rhs)
    {
        if (!node.IsSequence() || node.size() != 3)
        {
            return false;
        }

        rhs.x = node[0].as<double>();
        rhs.y = node[1].as<double>();
        rhs.z = node[2].as<double>();
        return true;
    }
};

template <> struct convert<Power>
{
    static bool decode(const Node &node, Power &rhs)
    {
        rhs.name = node["name"].as<std::string>();
        rhs.damage = node["damage"].as<int>();
        return true;
    }
};

template <> struct convert<Monster>
{
    static bool decode(const Node &node, Monster &rhs)
    {
        rhs.position = node["position"].as<Vec3>();
        rhs.name = node["name"].as<std::string>();
        const YAML::Node &powers = node["powers"];
        for (unsigned i = 0; i < powers.size(); i++)
        {
            rhs.powers.emplace_back(powers[i].as<Power>());
        }
        return true;
    }
};

} // namespace YAML

// https://github.com/jbeder/yaml-cpp/wiki/Tutorial
TEST(yaml, read_config_file)
{
    YAML::Node doc = YAML::LoadFile("./data/monsters.yaml");
    for (unsigned i = 0; i < doc.size(); i++)
    {
        std::cout << doc[i] << "\n";

        Monster monster = doc[i].as<Monster>();
        std::cout << rang::style::bold << rang::fg::gray << rang::bg::red << monster.name
                  << rang::style::reset << '\n';
    }
}
