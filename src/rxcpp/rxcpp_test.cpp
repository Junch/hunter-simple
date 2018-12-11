#include <array>
#include <gtest/gtest.h>
#include <random>
#include <rxcpp/rx.hpp>

///////////////////////////////////////////////////////////////////////////////
// Title: Streaming values from C++ containers
// Book: C++ Reactive Programming
///////////////////////////////////////////////////////////////////////////////
TEST(rxcpp, container)
{
    std::array<int, 3> a{1, 2, 3};
    auto values = rxcpp::observable<>::iterate(a);
    values.subscribe([](int v) { printf("OnNext: %d\n", v); }, []() { printf("OnCompleted\n"); });
}

///////////////////////////////////////////////////////////////////////////////
// Title: Creating Observables from scratch
// Book: C++ Reactive Programming
///////////////////////////////////////////////////////////////////////////////
TEST(rxcpp, scratch)
{
    auto ints = rxcpp::observable<>::create<int>([](rxcpp::subscriber<int> s) {
        s.on_next(1);
        s.on_next(4);
        s.on_next(9);
        s.on_completed();
    });

    ints.subscribe([](int v) { printf("OnNext: %d\n", v); }, []() { printf("OnCompleted\n"); });
}

std::string get_pid() {
    std::stringstream s;
    s << std::this_thread::get_id();
    return s.str();
}

///////////////////////////////////////////////////////////////////////////////
// Title: Creating Observables from scratch
// Web: http://reactivex.io/RxCpp/classrxcpp_1_1observable.html#a225ad69622b7c80328bae63232de84b6
///////////////////////////////////////////////////////////////////////////////
TEST(rxcpp, subscribe_vs_observe)
{
    printf("[thread %s] Start task\n", get_pid().c_str());
    {
        auto values = rxcpp::observable<>::range(1, 3).
            map([](int v){
                printf("-[thread %s] Emit value %d\n", get_pid().c_str(), v);
                return v;
            });
        values.
            subscribe_on(rxcpp::synchronize_new_thread()).
            as_blocking().
            subscribe(
                [](int v){printf("-[thread %s] OnNext: %d\n", get_pid().c_str(), v);},
                [](){printf("-[thread %s] OnCompleted\n", get_pid().c_str());});
    }

    printf("################################\n");

    {
        auto values = rxcpp::observable<>::range(1, 3).
            map([](int v){
                printf("[thread %s] Emit value %d\n", get_pid().c_str(), v);
                return v;
            });
        values.
            observe_on(rxcpp::synchronize_new_thread()).
            as_blocking().
            subscribe(
                [](int v){printf("[thread %s] OnNext: %d\n", get_pid().c_str(), v);},
                [](){printf("[thread %s] OnCompleted\n", get_pid().c_str());});
    }

    printf("################################\n");

    {
        auto values = rxcpp::observable<>::range(1, 3).
            subscribe_on(rxcpp::synchronize_new_thread()).
            map([](int v){
                printf("#[thread %s] Emit value %d\n", get_pid().c_str(), v);
                return v;
            });
        values.
            observe_on(rxcpp::synchronize_new_thread()).
            as_blocking().
            subscribe(
                [](int v){printf("#[thread %s] OnNext: %d\n", get_pid().c_str(), v);},
                [](){printf("#[thread %s] OnCompleted\n", get_pid().c_str());});
    }

    printf("[thread %s] Finish task\n", get_pid().c_str());
}

TEST(rxcpp, blocking)
{
    auto values = rxcpp::observable<>::range(1, 3).as_blocking();
    auto first = values.first();
    printf("first = %d\n", first);
}


TEST(rxcpp, timer_timepoint)
{
    auto start = std::chrono::steady_clock::now() + std::chrono::milliseconds(5000);
    auto values = rxcpp::observable<>::timer(start);
    values.
        subscribe(
            [](int v){printf("OnNext: %d\n", v);},
            [](){printf("OnCompleted\n");});
}

TEST(rxcpp, timer_timepoint_threaded)
{
    printf("[thread %s] Start task\n", get_pid().c_str());
    auto scheduler = rxcpp::observe_on_new_thread();
    auto start = scheduler.now() + std::chrono::milliseconds(1);
    auto values = rxcpp::observable<>::timer(start, scheduler);
    values.
        as_blocking().
        subscribe(
            [](int v){printf("[thread %s] OnNext: %d\n", get_pid().c_str(), v);},
            [](){printf("[thread %s] OnCompleted\n", get_pid().c_str());});
    printf("[thread %s] Finish task\n", get_pid().c_str());
}

TEST(rxcpp, timer_duration)
{
    auto period = std::chrono::milliseconds(5000);
    auto values = rxcpp::observable<>::timer(period);
    values.
        subscribe(
            [](int v){printf("OnNext: %d\n", v);},
            [](){printf("OnCompleted\n");});
}

TEST(rxcpp, connect)
{
    auto values = rxcpp::observable<>::interval(std::chrono::milliseconds(50), rxcpp::observe_on_new_thread()).
        take(5).
        publish();
    // Subscribe from the beginning
    values.subscribe(
        [](long v){printf("[1] OnNext: %ld\n", v);},
        [](){printf("[1] OnCompleted\n");});
    // Another subscription from the beginning
    values.subscribe(
        [](long v){printf("[2] OnNext: %ld\n", v);},
        [](){printf("[2] OnCompleted\n");});
    // Start emitting
    values.connect();

    // Wait before subscribing
    rxcpp::observable<>::timer(std::chrono::milliseconds(75)).subscribe([&](long){
        values.subscribe(
            [](long v){printf("[3] OnNext: %ld\n", v);},
            [](){printf("[3] OnCompleted\n");});
    });

    values.as_blocking().subscribe();
}

///////////////////////////////////////////////////////////////////////////////
// Title: Concatenating Observerable Streams
// Book: C++ Reactive Programming
///////////////////////////////////////////////////////////////////////////////
TEST(rxcpp, concat)
{
    auto values = rxcpp::observable<>::range(1);
    auto s1 = values.take(3).map([](int prime) { return 2 * prime; });
    auto s2 = values.take(3).map([](int prime) { return prime * prime; });
    s1.concat(s2).subscribe([](int i) { printf("OnNext: %d\n", i); }, []() { printf("OnCompleted\n"); });
}

namespace
{

static void sleep(int milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

class FrequencyMeter
{
    mutable std::mt19937 gen;
    mutable std::uniform_int_distribution<int> dis;
    mutable std::mutex m;

  public:
    FrequencyMeter() : dis(0, 3), gen(std::random_device()())
    {
    }

    inline int Hz() const
    {
        std::lock_guard<std::mutex> lock(m);
        return 60 + dis(gen);
    }
};

void rxcpp_tick()
{
    FrequencyMeter FM;

    std::atomic<long> pending(2);

    // schedule everything on the same event loop thread.
    auto scheduler = rxcpp::schedulers::make_same_worker(rxcpp::schedulers::make_event_loop().create_worker());
    auto coordination = rxcpp::identity_one_worker(scheduler);

    auto measure = rxcpp::observable<>::interval(
                       // when to start
                       scheduler.now() + std::chrono::milliseconds(250),
                       // measurement frequency
                       std::chrono::milliseconds(250), coordination)
                       // take Hz values instead of a counter
                       .map([&FM](int) { return FM.Hz(); });

    auto measure_subscription = measure.subscribe([](int val) { std::cout << val << std::endl; });

    auto ticker = rxcpp::observable<>::interval(scheduler.now() + std::chrono::milliseconds(500),
                                                std::chrono::milliseconds(500), coordination);

    ticker.take(10).subscribe([](int val) { std::cout << "tick " << val << std::endl; },
                              [&]() {
                                  --pending; // take completed the ticker
                              });

    // schedule the cout on the same worker to keep it from merging with the other cout calls.
    scheduler.create_worker().schedule(scheduler.now() + std::chrono::seconds(2), [&](const rxcpp::schedulers::schedulable &) {
        std::cout << "Canceling measurement ..." << std::endl;
        measure_subscription.unsubscribe(); // cancel measurement
        --pending;                          // signal measurement canceled
    });

    while (pending)
    {
        sleep(1000); // wait for ticker and measure to finish
    }
}
} // namespace

///////////////////////////////////////////////////////////////////////////////
// Title: Creating Observables from scratch
// Web: https://github.com/d-led/ticker
///////////////////////////////////////////////////////////////////////////////
TEST(rxcpp, ticker)
{
    rxcpp_tick();
}
