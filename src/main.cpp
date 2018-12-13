#include <gtest/gtest.h>
#include <gflags/gflags.h>

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    gflags::SetVersionString("1.0.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    return RUN_ALL_TESTS();
}
