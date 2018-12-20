#include <gtest/gtest.h>
#include <glog/logging.h>

// https://blog.csdn.net/u012348774/article/details/80558533

TEST(glog, simple)
{
    LOG(INFO) << "This is INFO";
    LOG(WARNING) << "This is WARNING";
    LOG(ERROR) << "This is Error";
}
