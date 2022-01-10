#include "test_util.h"

#include <vector>
#include <thread>

#include "los/logs.h"

extern bool b_app_start;

static void SleepThread(int sleep_time_ns)
{
    size_t idx = 0;
    auto start_time = std::chrono::steady_clock::now();
    while (b_app_start)
    {
        ++idx;
        auto cur_time = std::chrono::steady_clock::now();
        std::chrono::nanoseconds before_time = cur_time - start_time;
        los::logs::DefaultLogger()->Log(los::logs::kInfo, false, __FILE__, __LINE__, "%zu, before, %lld", idx, before_time.count());

        std::this_thread::sleep_for(std::chrono::nanoseconds(sleep_time_ns));

        cur_time = std::chrono::steady_clock::now();
        std::chrono::nanoseconds after_time = cur_time - start_time;
        los::logs::DefaultLogger()->Log(los::logs::kInfo, false, __FILE__, __LINE__, "%zu, after, %lld, %lld",
            idx, after_time.count(), after_time.count() - before_time.count());

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void TestSleepTime(int argc, char **argv)
{
    int sleep_time_ns = 1;
    if (argc >= 3)
    {
        sleep_time_ns = atoi(argv[2]);
    }
    else
    {
        printf("Input sleep ns:");
        scanf("%d", &sleep_time_ns);
    }
    if (sleep_time_ns < 1) sleep_time_ns = 1;

    SleepThread(sleep_time_ns);
}

void TestMultiThreadSleepTime(int argc, char **argv)
{
    int thread_cnt = 1;
    if (argc >= 3)
    {
        thread_cnt = atoi(argv[2]);
    }
    else
    {
        printf("Input number of thread:");
        scanf("%d", &thread_cnt);
    }
    if (thread_cnt < 1) thread_cnt = 1;

    int sleep_time_ns = 1;
    if (argc >= 4)
    {
        sleep_time_ns = atoi(argv[3]);
    }
    else
    {
        printf("Input sleep ns:");
        scanf("%d", &sleep_time_ns);
    }
    if (sleep_time_ns < 1) sleep_time_ns = 1;

    std::vector<std::thread> threads;
    for (int i = 0; i < thread_cnt; ++i)
    {
        threads.emplace_back(&SleepThread, sleep_time_ns);
    }

    for (auto &&thd : threads)
    {
        if (thd.joinable())
        {
            thd.join();
        }
    }
}
