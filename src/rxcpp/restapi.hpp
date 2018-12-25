#pragma once

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/json.h>

using namespace utility;                    // Common utilities like string conversions
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

#include "rxcpp/rx.hpp"
namespace Rx {
    using namespace rxcpp;
    using namespace rxcpp::sources;
    using namespace rxcpp::operators;
    using namespace rxcpp::util;
}
using namespace Rx;

#include <nlohmann/json.hpp>
using nlohmann::json;

template<class T>
struct RestApi {
    http_client client;
    
    RestApi(const uri &base_uri) : client(base_uri) {}
    
    observable<string_t> getString(const string_t &path_query_fragment) {
        return observable<>::create<string_t>(
        [&](subscriber<string_t> s){
            client
            .request(methods::GET, path_query_fragment)
            .then([](http_response response) -> pplx::task<string_t> {
                return response.extract_string();
            })
            .then([&](pplx::task<string_t> previousTask) {
                try {
                    string_t const & v = previousTask.get();
                    s.on_next(v);
                    s.on_completed();
                } catch (http_exception const & e) {
                    s.on_error(std::current_exception());
                }
            })
            .wait();
        });
    }
    
    observable<T> getObject(const string_t &path_query_fragment) {
        return observable<>::create<T>(
        [&](subscriber<T> s){
            client
            .request(methods::GET, path_query_fragment)
            .then([](http_response response) -> pplx::task<string_t> {
                return response.extract_string();
            })
            .then([&](pplx::task<string_t> previousTask) {
                try {
                    string_t const & v = previousTask.get();
                    json j = json::parse(v);
                    T t = j;
                    s.on_next(t);
                    s.on_completed();
                } catch (http_exception const & e) {
                    s.on_error(std::current_exception());
                }
            })
            .wait();
        });
    }
    
    observable<T> getArray(const string_t &path_query_fragment) {
        return observable<>::create<T>(
        [&](subscriber<T> s){
            client
            .request(methods::GET, path_query_fragment)
            .then([](http_response response) -> pplx::task<string_t> {
                return response.extract_string();
            })
            .then([&](pplx::task<string_t> previousTask) {
                try {
                    string_t const & v = previousTask.get();
                    json j = json::parse(v);
                    std::vector<T> vec = j;
                    for(const auto& t : vec)
                        s.on_next(t);
                    s.on_completed();
                } catch (http_exception const & e) {
                    s.on_error(std::current_exception());
                }
            })
            .wait();
        });
    }
 
    observable<string_t> createObject(const string_t& url, const T& obj) {
        return observable<>::create<string_t>(
        [&](subscriber<string_t> s){
            json j = obj;
            client
            .request(methods::POST, url, j.dump(), U("application/json"))
            .then([](http_response response) -> pplx::task<string_t> {
                return response.extract_string();
            })
            .then([&](pplx::task<string_t> previousTask) {
                try {
                    string_t const & v = previousTask.get();
                    s.on_next(v);
                    s.on_completed();
                } catch (http_exception const & e) {
                    s.on_error(std::current_exception());
                }
            })
            .wait();
        });
    }
    
    observable<string_t> updateObject(const string_t& url, const T& obj) {
        return observable<>::create<string_t>(
        [&](subscriber<string_t> s){
            json j = obj;
            client
            .request(methods::PUT, url, j, U("application/json"))
            .then([](http_response response) -> pplx::task<string_t> {
                return response.extract_string();
            })
            .then([&](pplx::task<string_t> previousTask) {
                try {
                    string_t const & v = previousTask.get();
                    s.on_next(v);
                    s.on_completed();
                } catch (http_exception const & e) {
                    s.on_error(std::current_exception());
                }
            })
            .wait();
        });
    }
    
    observable<string_t> deleteObject(const string_t& url) {
        return observable<>::create<string_t>(
        [&](subscriber<string_t> s){
            client
            .request(methods::DEL, url)
            .then([](http_response response) -> pplx::task<string_t> {
                return response.extract_string();
            })
            .then([&](pplx::task<string_t> previousTask) {
                try {
                    string_t const & v = previousTask.get();
                    s.on_next(v);
                    s.on_completed();
                } catch (http_exception const & e) {
                    s.on_error(std::current_exception());
                }
            })
            .wait();
        });
    }
};
