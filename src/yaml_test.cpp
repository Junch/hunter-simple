#include "yaml-cpp/yaml.h"
#include "gtest/gtest.h"

TEST(yaml, simple)
{
    YAML::Emitter out;
    out << "Hello, World!";
    ASSERT_STREQ("Hello, World!", out.c_str());
}
