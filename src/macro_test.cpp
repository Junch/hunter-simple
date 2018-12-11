#include <gtest/gtest.h>
#include <map>
#include <functional>
#include <string>

#define WIDEN(x) L ## #x
#define CAT(x) on ## x ## Changed
#define CONFIG2METHOD(x) { WIDEN(x), [](){ CAT(x)(); } }

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
        std::wcout << item.first << " ==> ";
        item.second();
    }
}
