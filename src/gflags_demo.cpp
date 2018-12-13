#include <gflags/gflags.h>
#include <gtest/gtest.h>
#include <iostream>

static bool ValidatePort(const char *flagname, int value)
{
    if (value > 0 && value < 32768)
        return true;
    printf("Invalid value for --%s: %d\n", flagname, (int)value);
    return false;
}

DEFINE_string(bind_ip, "localhost", "set bind ip");
DEFINE_int32(listen_port, 8086, "set listen port");
DEFINE_validator(listen_port, &ValidatePort);
DEFINE_bool(is_master, false, "set if is master");

// http://dreamrunner.org/blog/2014/03/09/gflags-jian-ming-shi-yong/
// https://gflags.github.io/gflags/
// https://blog.csdn.net/chang_mu/article/details/75570782
TEST(gflags, simple)
{
    std::cout << "bind_ip:" << FLAGS_bind_ip << '\n';
    std::cout << "listen_port:" << FLAGS_listen_port << '\n';
    std::cout << "is_master:" << FLAGS_is_master << '\n';
}
