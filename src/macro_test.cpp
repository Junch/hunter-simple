#include <functional>
#include <gtest/gtest.h>
#include <map>
#include <string>

#define WIDEN(x) L## #x
#define CAT(x) on##x##Changed
#define CONFIG2METHOD(x)                                                                                                       \
    {                                                                                                                          \
        WIDEN(x), []() { CAT(x)(); }                                                                                           \
    }

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
    // clang-format off
    std::map<std::wstring, std::function<void()>> configs = 
    {
        CONFIG2METHOD(DirectoryUri),
        CONFIG2METHOD(PrimaryServerName)
    };
    // clang-format on

    ASSERT_EQ(2, configs.size());
    for (auto& item: configs)
    {
        std::wcout << item.first << " ==> ";
        item.second();
    }
}

namespace wrapper
{

class IPerson
{
  public:
    virtual std::string getName() = 0;
    virtual int getAge() = 0;
};

class PersonImpl
{
  public:
    virtual std::string getName() { return "Tom"; }
    int getAge() { return 20; }
};

class PersonWrapper : IPerson
{
  public:
    PersonWrapper(std::shared_ptr<PersonImpl> person);
    std::string getName();
    int getAge();

  private:
    std::shared_ptr<PersonImpl> _person;
};

PersonWrapper::PersonWrapper(std::shared_ptr<PersonImpl> person) : _person(person)
{
}

#define PERSON_PROPERTY_WRAPPER(x, y)                                                                                          \
    x PersonWrapper::get##y() { return _person->get##y(); }

PERSON_PROPERTY_WRAPPER(std::string, Name);
PERSON_PROPERTY_WRAPPER(int, Age);

} // namespace wrapper
