#include <stdio.h>
#include "curl/curl.h"
#include "gtest/gtest.h"

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

TEST(Curl, writeData) {
    CURL* curl = curl_easy_init();
    std::string readBuffer;

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://www.bing.com");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        ASSERT_EQ(res, CURLE_OK);
        ASSERT_TRUE(readBuffer.length() > 0);
    }
}
