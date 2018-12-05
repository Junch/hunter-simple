#include <array>
#include <gtest/gtest.h>
#include <iostream>
#include <pplx/pplxtasks.h>
#include <string>
#include <thread>
using namespace std::chrono_literals;

pplx::task<std::wstring> write_to_string()
{
    // Create a shared pointer to a string that is
    // assigned to and read by multiple tasks.
    // By using a shared pointer, the string outlives
    // the tasks, which can run in the background after
    // this function exits.
    auto s = std::make_shared<std::wstring>(L"Value 1");

    return pplx::create_task([s] {
               std::thread::id this_id = std::this_thread::get_id();
               std::wcout << std::hex << this_id << L" Current value: " << *s << '\n';
               *s = L"Value 2";
           })
        .then([s] {
            std::thread::id this_id = std::this_thread::get_id();
            std::wcout << std::hex << this_id << L" Current value: " << *s << '\n';
            *s = L"Value 3";
            return *s;
        });
}

TEST(pplxtask, simple)
{
    // Create a chain of tasks that work with a string.
    auto t = write_to_string();

    // Wait for the tasks to finish and print the result.
    auto finalValue = t.get();
    std::thread::id this_id = std::this_thread::get_id();
    std::wcout << std::hex << this_id << L" Final value: " << finalValue << '\n';
}

// https://www.jianshu.com/p/171b8c92bd4e
TEST(pplxtask, all)
{
    std::array<pplx::task<void>, 3> tasks = {pplx::create_task([] { std::cout << "Hello from taskA." << '\n'; }),
                                             pplx::create_task([] { std::cout << "Hello from taskB." << '\n'; }),
                                             pplx::create_task([] { std::cout << "Hello from taskC." << '\n'; })};

    auto joinTask = pplx::when_all(std::begin(tasks), std::end(tasks));

    std::cout << "Hello from the joining thread." << std::endl;
    joinTask.wait();
}

// https://www.jianshu.com/p/171b8c92bd4e
TEST(pplxtask, cancell)
{
    pplx::cancellation_token_source cts;
    std::cout << "Creating task..." << '\n';

    auto task = pplx::create_task([cts] {
        bool moreToDo = true;
        while (moreToDo)
        {
            if (cts.get_token().is_canceled())
            {
                return;
            }
            else
            {
                moreToDo = []() -> bool {
                    std::cout << "Performing work..." << '\n';
                    std::this_thread::sleep_for(250ms);
                    return true;
                }();
            }
        }
    });

    std::this_thread::sleep_for(1000ms);

    std::cout << "Canceling task..." << '\n';
    cts.cancel();
    std::cout << "Waiting for task to complete..." << '\n';
    task.wait();
    std::cout << "Done." << '\n';
}
