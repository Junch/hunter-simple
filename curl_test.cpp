#include <stdio.h>
#include <array>
#include <string>
#include <memory>
#include <map>
#include "curl/curl.h"
#include "gtest/gtest.h"
#include "string_format.h"

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(contents, size, nmemb, (FILE *)stream);
    return written;
}

TEST(Curl, simple) {
    CURL* curl = curl_easy_init();
    const char* filename = "huiluo2.jpg";
    FILE* file = fopen(filename, "wb");

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://cmbu-ad.cisco.com/photo/huiluo2.jpg");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
        CURLcode res = curl_easy_perform(curl);
        fclose(file);

        int httpCode = 0;
        res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        if (res != CURLE_OK || httpCode != 200) {
            remove(filename);
            printf("Failed to download the file: curlCode=%d, httpCode=%d\n", res, httpCode);
        }

        curl_easy_cleanup(curl);
    }
}

// https://gist.github.com/clemensg/4960504

#define MAX_WAIT_MSECS 30*1000 /* Wait max. 30 seconds */

struct Node {
    Node(const char* name){
        url = string_format("http://cmbu-ad.cisco.com/photo/%s.jpg", name);
        filename = string_format("%s.jpg", name);
        file = fopen(filename.c_str(), "wb");
    }

    FILE* file;
    std::string url;
    std::string filename;
};

std::array<const char*, 21> names = {
  "chaowei", "cheluo", "chenxzha", "cyshen", "gamao",
  "huiluo", "jizh", "jinkgao", "juyan", "juphan",
  "liatang", "libliang","lilli", "mingwa", "nhaotian",
  "qiozhang", "quxie", "stigao", "wazhu", "xumei", "jzhichen"
};

static size_t cb(char *contents, size_t size, size_t nmemb, void *user)
{
    Node* node = reinterpret_cast<Node*>(user);
    FILE* file = node->file;
    size_t written = fwrite(contents, size, nmemb, file);
    return written;
}

std::map<CURL*, std::shared_ptr<Node>> gActiveTransfers;

static void init(CURLM *cm, const char* name)
{
    std::shared_ptr<Node> pNode = std::make_shared<Node>(name);
    CURL *eh = curl_easy_init();
    curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, cb);
    curl_easy_setopt(eh, CURLOPT_HEADER, 0L);
    curl_easy_setopt(eh, CURLOPT_URL, pNode->url.c_str());
    curl_easy_setopt(eh, CURLOPT_PRIVATE, pNode->url.c_str());
    curl_easy_setopt(eh, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(eh, CURLOPT_WRITEDATA, pNode.get());
    curl_multi_add_handle(cm, eh);
    gActiveTransfers[eh] = pNode;
}

TEST(Curl, multi) {
    curl_global_init(CURL_GLOBAL_ALL);

    CURLM* cm = curl_multi_init();
    for (int i = 0; i < names.size(); ++i) {
        init(cm, names[i]);
        for (int j=1; j<10; ++j) {
            std::string myName = string_format("%s%d", names[i], j);
            init(cm, myName.c_str());
        }
    }

    int still_running = 0;
    curl_multi_perform(cm, &still_running);

    do {
        int numfds = 0;
        int res = curl_multi_wait(cm, NULL, 0, MAX_WAIT_MSECS, &numfds);
        if (res != CURLM_OK) {
            fprintf(stderr, "error: curl_multi_wait() returned %d\n", res);
            FAIL();
        }

        curl_multi_perform(cm, &still_running);
    } while (still_running);

    int msgs_left = 0;
    while (CURLMsg *msg = curl_multi_info_read(cm, &msgs_left)) {
        if (msg->msg == CURLMSG_DONE) {

            CURLcode return_code = msg->data.result;
            if (return_code != CURLE_OK) {
                fprintf(stderr, "CURL error code: %d\n", msg->data.result);
                continue;
            }

            CURL *eh = msg->easy_handle;
            int http_status_code = 0;
            const char* szUrl = NULL;

            curl_easy_getinfo(eh, CURLINFO_RESPONSE_CODE, &http_status_code);
            curl_easy_getinfo(eh, CURLINFO_PRIVATE, &szUrl);

            std::shared_ptr<Node> node = nullptr;
            auto iter = gActiveTransfers.find(eh);
            if (iter != gActiveTransfers.end()) {
                node = iter->second;
                gActiveTransfers.erase(eh);
            }
            else {
                fprintf(stderr, "The internal data object was not found in the map");
                continue;
            }

            fclose(node->file);

            if (http_status_code == 200) {
                printf("200 OK for %s\n", szUrl);
            }
            else {
                remove(node->filename.c_str());
                fprintf(stderr, "GET of %s returned http status code %d\n", szUrl, http_status_code);
            }

            curl_multi_remove_handle(cm, eh);
            curl_easy_cleanup(eh);
        }
        else {
            fprintf(stderr, "error: after curl_multi_info_read(), CURLMsg=%d\n", msg->msg);
        }
    }

    curl_multi_cleanup(cm);
}