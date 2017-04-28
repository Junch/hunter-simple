#include <stdio.h>
#include <array>
#include <string>
#include <memory>
#include <map>
#include <thread>
#include "curl/curl.h"
#include "gtest/gtest.h"
#include "string_format.h"

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(contents, size, nmemb, (FILE *)stream);
    return written;
}

struct myprogress {
    double lastruntime;
    CURL *curl;
};

static int xferinfo(void *p,
                    curl_off_t dltotal, curl_off_t dlnow,
                    curl_off_t ultotal, curl_off_t ulnow)
{
    struct myprogress *myp = (struct myprogress *)p;
    CURL *curl = myp->curl;

    fprintf(stderr, "UP: %" CURL_FORMAT_CURL_OFF_T " of %" CURL_FORMAT_CURL_OFF_T
            "  DOWN: %" CURL_FORMAT_CURL_OFF_T " of %" CURL_FORMAT_CURL_OFF_T
            "\r",
            ulnow, ultotal, dlnow, dltotal);

    return 0;
}

void curl_thread_simple(){
	curl_version_info_data *ver = curl_version_info(CURLVERSION_NOW);
    printf("CURL Ver: %s\n", ver->version);

    const char* filename = "survival_download.mp3";
    const char* url = "http://downloads.bbc.co.uk/learningenglish/features/6min/170427_6min_engl_miraculous_survival_download.mp3";
    FILE* file = fopen(filename, "wb");
    myprogress prog;

    std::unique_ptr<CURL, void(*)(CURL*)> eh_ptr(curl_easy_init(), curl_easy_cleanup);
    if (eh_ptr) {
        CURL* eh = eh_ptr.get();
        curl_easy_setopt(eh, CURLOPT_URL, url);
        curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(eh, CURLOPT_WRITEDATA, file);
        curl_easy_setopt(eh, CURLOPT_CONNECTTIMEOUT_MS, 6000L);
        curl_easy_setopt(eh, CURLOPT_NOPROGRESS, 0); 
        curl_easy_setopt(eh, CURLOPT_XFERINFOFUNCTION, xferinfo);
        curl_easy_setopt(eh, CURLOPT_XFERINFODATA, &prog);
        CURLcode res = curl_easy_perform(eh);
        fclose(file);

        int httpCode = 0;
        curl_easy_getinfo(eh, CURLINFO_RESPONSE_CODE, &httpCode);
        if (res != CURLE_OK) {
            remove(filename);
            printf("Failed to download the file: curlCode=%d: %s\n", res, curl_easy_strerror(res));
        }else if(httpCode != 200) {
            remove(filename);
            printf("Failed to download the file: httpCode=%d\n", httpCode);
        }
    }
}

TEST(curl, thread_simple) {
	std::thread trd(curl_thread_simple);
	trd.join();
}
