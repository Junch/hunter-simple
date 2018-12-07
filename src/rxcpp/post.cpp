#include "post.hpp"
#include <boost/algorithm/string/replace.hpp>
using namespace std;

void to_json(json &j, const Post &p)
{
    j = json{{"userId", p.userId}, {"id", p.id}, {"title", p.title}, {"body", p.body}};
}

void from_json(const json &j, Post &p)
{
    p.userId = j.at("userId").get<int>();
    p.id = j.at("id").get<int>();
    p.title = j.at("title").get<string>();
    p.body = boost::algorithm::replace_all_copy(j.at("body").get<string>(), "\n", "\\n");
}

std::ostream &operator<<(std::ostream &out, const Post &p)
{
    cout << "Post {userId = " << p.userId << ", id = " << p.id << ", title = \"" << p.title << "\", body = \"" << p.body
         << "\"}";
    return out;
}
