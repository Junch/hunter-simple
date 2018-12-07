#include <gtest/gtest.h>
#include "post.hpp"
#include "restapi.hpp"

class restapi : public testing::Test
{
  public:
    void SetUp() override {}
    void TearDown() override {}

  protected:
    RestApi<Post> api{U("https://jsonplaceholder.typicode.com/")};
};

// 使用 RxCpp + C++ REST SDK 调用 REST API
// https://www.cnblogs.com/zwvista/p/9607303.html

TEST_F(restapi, getString)
{
    api.getString(U("posts/1")).subscribe([](const string_t &v) { std::cout << v << '\n'; });
}

TEST_F(restapi, getObject)
{
    api.getObject(U("posts/1")).subscribe([](const Post &v) { std::cout << v << '\n'; });
}

TEST_F(restapi, getArray)
{
    api.getArray(U("posts")).take(2).subscribe([](const Post &v) { std::cout << v << '\n'; });
}

TEST_F(restapi, createObject)
{
    Post o;
    o.id = 0;
    o.userId = 101;
    o.title = U("test title");
    o.body = U("test body");
    api.getArray(U("posts")).take(2).subscribe([](const Post &v) { std::cout << v << '\n'; });
}

TEST_F(restapi, updateObject)
{
    Post o;
    o.id = 0;
    o.userId = 101;
    o.title = U("test title");
    o.body = U("test body");
    api.updateObject(U("posts/1"), o).subscribe([](string_t v) { std::cout << v << '\n'; });
}

TEST_F(restapi, deleteObject)
{
    api.deleteObject(U("posts/1")).subscribe([](string_t v) { std::cout << v << '\n'; });
}
