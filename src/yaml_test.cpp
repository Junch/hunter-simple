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

void operator>>(const YAML::Node &node, Vec3 &v)
{
    v.x = node[0].as<float>();
    v.y = node[1].as<float>();
    v.z = node[2].as<float>();
}

void operator>>(const YAML::Node &node, Power &power)
{
    power.name = node["name"].as<std::string>();
    power.damage = node["damage"].as<int>();
}

void operator>>(const YAML::Node &node, Monster &monster)
{
    node["position"] >> monster.position;
    monster.name = node["name"].as<std::string>();
    const YAML::Node &powers = node["powers"];
    for (unsigned i = 0; i < powers.size(); i++)
    {
        Power power;
        powers[i] >> power;
        monster.powers.push_back(power);
    }
}

// https://github.com/jbeder/yaml-cpp/wiki/Tutorial
TEST(yaml, read_config_file)
{
    YAML::Node doc = YAML::LoadFile("./data/monsters.yaml");
    for (unsigned i = 0; i < doc.size(); i++)
    {
        std::cout << doc[i] << "\n";

        Monster monster;
        doc[i] >> monster;
        std::cout << rang::style::bold << rang::fg::gray << rang::bg::red << monster.name
                  << rang::style::reset << '\n';
    }
}
