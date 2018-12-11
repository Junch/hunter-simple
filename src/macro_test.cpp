#include <gtest/gtest.h>
#include <map>
#include <functional>
#include <string>

#define CAT(a) on ## a ## Changed
#define CONFIG2METHOD(x) { L#x, [](){ CAT(x)(); } }

void onDirectoryUriChanged()
{
    printf("onDirectoryUriChanged\n");
}

void onPrimaryServerNameChanged()
{
    printf("onPrimaryServerNameChanged\n");
}

TEST(macro, concat)
{
    std::map<std::wstring, std::function<void()>> configs = 
    {
        CONFIG2METHOD(DirectoryUri),
        CONFIG2METHOD(PrimaryServerName)
    };

    ASSERT_EQ(2, configs.size());
    for (auto& item: configs)
    {
        item.second();
    }
}
