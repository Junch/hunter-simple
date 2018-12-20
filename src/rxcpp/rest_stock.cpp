#include <gtest/gtest.h>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <glog/logging.h>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

std::map<utility::string_t, utility::string_t> dictionary = 
{
    {"SH510300", "300ETF"},
    {"SH601318", "中国平安"}
};

void display_json( json::value const & jvalue, utility::string_t const & prefix)
{
   std::cout << prefix << jvalue.serialize() << '\n';
}
 
void handle_get(http_request request)
{
   LOG(INFO) << "handle GET";
 
   auto answer = json::value::object();
   for (auto const & p : dictionary)
   {
      answer[p.first] = json::value::string(p.second);
   }
 
   display_json(json::value::null(), "R: ");
   display_json(answer, "S: ");
 
   request.reply(status_codes::OK, answer);
}

///////////////////////////////////////////////////////////////////////////////
// Title: Revisited: Full-fledged client-server example with C++ REST SDK 2.10
// https://mariusbancila.ro/blog/2017/11/19/revisited-full-fledged-client-server-example-with-c-rest-sdk-2-10/
///////////////////////////////////////////////////////////////////////////////

TEST(rest, stock)
{
    http_listener listener("http://127.0.0.1:8080/stockData");

    listener.support(methods::GET, handle_get);

    try
    {
        listener.open().then(
            [&listener]() {
                LOG(INFO) << "starting to listen";
            }).wait();

        while (true);
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << '\n';
    }
}
