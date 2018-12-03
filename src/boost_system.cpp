#include <boost/system/error_code.hpp>
#include <gtest/gtest.h>
#include <iostream>

using namespace boost::system;

///////////////////////////////////////////////////////////////////////////////
// build with -std=c++14 will cause link error. Solution is to add two macros:
//    -DBOOST_SYSTEM_NO_DEPRECATED
//    -DBOOST_ERROR_CODE_HEADER_ONLY
// Web: https://github.com/boostorg/system/issues/26
///////////////////////////////////////////////////////////////////////////////

void fail(error_code &ec)
{
    ec = errc::make_error_code(errc::not_supported);
}

TEST(boost, error_code)
{
    error_code ec;
    fail(ec);
    std::cout << ec.value() << '\n';
}
