#include <gtest/gtest.h>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <glog/logging.h>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

#define TRACE_ACTION(a, k, v) std::cout << a << " (" << k << ", " << v << ")\n"

// clang-format off
std::map<utility::string_t, utility::string_t> dictionary = 
{
    {"SH510300", "300ETF"},
    {"SH601318", "中国平安"}
};
// clang-format on

void display_json(json::value const &jvalue, utility::string_t const &prefix)
{
    std::cout << prefix << jvalue.serialize() << '\n';
}

void DUMP(http_request message)
{
    ucout << U("Method: ") << message.method() << std::endl;
    ucout << U("URI: ") << http::uri::decode(message.relative_uri().path()) << std::endl;
    ucout << U("Query: ") << http::uri::decode(message.relative_uri().query()) << std::endl << std::endl;
};

void handle_get(http_request request)
{
    LOG(INFO) << "handle GET";
    DUMP(request);

    auto answer = json::value::object();
    for (auto const &p : dictionary)
    {
        answer[p.first] = json::value::string(p.second);
    }

    display_json(json::value::null(), "R: ");
    display_json(answer, "S: ");

    request.reply(status_codes::OK, answer);
}

void handle_request(http_request request, std::function<void(json::value const &, json::value &)> action)
{
    DUMP(request);

    auto answer = json::value::object();

    request.extract_json()
        .then([&answer, &action](pplx::task<json::value> task) {
            try
            {
                auto const &jvalue = task.get();
                display_json(jvalue, "R: ");

                if (!jvalue.is_null())
                {
                    action(jvalue, answer);
                }
            }
            catch (http_exception const &e)
            {
                LOG(ERROR) << e.what();
            }
        })
        .wait();

    display_json(answer, "S: ");

    request.reply(status_codes::OK, answer);
}

///////////////////////////////////////////////////////////////////////////////
//  POST json data should be like ["SH510300", "SH601318", ...]
///////////////////////////////////////////////////////////////////////////////
void handle_post(http_request request)
{
    LOG(INFO) << "handle POST";

    handle_request(request, [](json::value const &jvalue, json::value &answer) {
        for (auto const &e : jvalue.as_array())
        {
            if (e.is_string())
            {
                auto key = e.as_string();
                auto pos = dictionary.find(key);

                if (pos == dictionary.end())
                {
                    answer[key] = json::value::string("<nil>");
                }
                else
                {
                    answer[pos->first] = json::value::string(pos->second);
                }
            }
        }
    });
}

///////////////////////////////////////////////////////////////////////////////
//  PUT json data should be like {"SZ000651": "格力电器"}
///////////////////////////////////////////////////////////////////////////////
void handle_put(http_request request)
{
    LOG(INFO) << "handle PUT";

    handle_request(request, [](json::value const &jvalue, json::value &answer) {
        for (auto const &e : jvalue.as_object())
        {
            if (e.second.is_string())
            {
                auto key = e.first;
                auto value = e.second.as_string();

                if (dictionary.find(key) == dictionary.end())
                {
                    TRACE_ACTION("added", key, value);
                    answer[key] = json::value::string("<put>");
                }
                else
                {
                    TRACE_ACTION("updated", key, value);
                    answer[key] = json::value::string("<updated>");
                }

                dictionary[key] = value;
            }
        }
    });
}

void handle_del(http_request request)
{
    LOG(INFO) << "handle DEL";

    handle_request(request, [](json::value const &jvalue, json::value &answer) {
        std::set<utility::string_t> keys;
        for (auto const &e : jvalue.as_array())
        {
            if (e.is_string())
            {
                auto key = e.as_string();

                auto pos = dictionary.find(key);
                if (pos == dictionary.end())
                {
                    answer[key] = json::value::string("<failed>");
                }
                else
                {
                    TRACE_ACTION("deleted", pos->first, pos->second);
                    answer[key] = json::value::string("<deleted>");
                    keys.insert(key);
                }
            }
        }

        for (auto const &key : keys)
            dictionary.erase(key);
    });
}

///////////////////////////////////////////////////////////////////////////////
// Title: Revisited: Full-fledged client-server example with C++ REST SDK 2.10
// Web: https://mariusbancila.ro/blog/2017/11/19/revisited-full-fledged-client-server-example-with-c-rest-sdk-2-10/
// Title: http://blog.51cto.com/wdx04/1727907
// Web: http://blog.51cto.com/wdx04/1727907
///////////////////////////////////////////////////////////////////////////////

TEST(rest, stock)
{
    http_listener listener("http://127.0.0.1:8080/stockData");

    listener.support(methods::GET, handle_get);
    listener.support(methods::POST, handle_post);
    listener.support(methods::PUT, handle_put);
    listener.support(methods::DEL, handle_del);

    try
    {
        listener.open().then([&listener]() { LOG(INFO) << "starting to listen"; }).wait();


        ucout << U("Press ENTER key to quit...") << std::endl;
        std::string line;
        std::getline(std::cin, line);
        listener.close().wait();
    }
    catch (const std::exception &e)
    {
        LOG(ERROR) << e.what();
    }
}
