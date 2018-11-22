#include "predicate.h"
#include <chrono>
#include <condition_variable>
#include <thread>

predicate::predicate(std::function<bool()> fxn, unsigned int timeoutMs)
    : _predicateFxn(fxn), _timeoutMs(timeoutMs)
{
}

bool predicate::evaluate()
{
    std::condition_variable cv;
    std::mutex mutex;

    bool timeout = false;
    bool completed = false;

    std::thread trd([&] {
        while (true)
        {
            {
                std::lock_guard<std::mutex> lock(mutex);
                if (timeout)
                {
                    return;
                }
            }

            if (_predicateFxn())
            {
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    completed = true;
                }

                cv.notify_one();
                return;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    bool ret = false;
    {
        std::unique_lock<std::mutex> lock(mutex);
        // https://zh.cppreference.com/w/cpp/thread/condition_variable/wait_for
        ret = cv.wait_for(lock, std::chrono::milliseconds(_timeoutMs),
                          [&completed] { return completed; });
    }

    if (!ret) // timeout
    {
        std::lock_guard<std::mutex> lock(mutex);
        timeout = true;
    }

    if (trd.joinable())
    {
        trd.join();
    }

    return ret; // false means timeout
}
