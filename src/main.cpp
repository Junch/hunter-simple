#include <gtest/gtest.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    gflags::SetVersionString("1.0.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = 1; // It will dump to console as well

    return RUN_ALL_TESTS();
}
