#include <gtest/gtest.h>

class MyTestCase : public testing::TestWithParam<int>
{
};

TEST_P(MyTestCase, MyTest)
{
    // https://github.com/abseil/googletest/blob/master/googletest/docs/advanced.md

    std::cout << "Example Test Param: " << GetParam() << '\n';

    std::string case_name = ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name();
    std::string test_name = case_name + "." + ::testing::UnitTest::GetInstance()->current_test_info()->name();

    std::cout << "Test Name: " << test_name << '\n';
}

INSTANTIATE_TEST_CASE_P(MyGroup, MyTestCase, testing::Range(0, 3), testing::PrintToStringParamName());
