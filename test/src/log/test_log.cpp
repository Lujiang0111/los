#include "test_log.h"
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <thread>
#include "los/logs.h"

constexpr size_t kLogSize = 10000000;

extern bool b_app_start;

static void LogThread(std::shared_ptr<los::logs::ILogger> logger, bool is_sync, double speed)
{
    auto last_time = std::chrono::steady_clock::now();
    double last_cnt = 0;
    size_t idx = 0;
    while (b_app_start)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto cur_time = std::chrono::steady_clock::now();
        last_cnt += (std::chrono::duration_cast<std::chrono::microseconds>(cur_time - last_time).count() / 1000000.0 * speed);
        while (last_cnt >= 1)
        {
            logger->Log(los::logs::kDebug, is_sync, true, __FILE__, __LINE__, "%zu", idx);
            logger->Log(los::logs::kInfo, is_sync, true, __FILE__, __LINE__, "%zu", idx);
            logger->Log(los::logs::kWarn, is_sync, true, __FILE__, __LINE__, "%zu", idx);
            logger->Log(los::logs::kError, is_sync, true, __FILE__, __LINE__, "%zu", idx);
            ++idx;
            --last_cnt;
        }
        last_time = cur_time;
    }
}

void TestSingleThreadLogging(int argc, char **argv)
{
    bool is_sync = false;
    if (argc >= 3)
    {
        is_sync = ('0' != argv[2][0]);
    }
    else
    {
        printf("Input is sync[0/1]:");
        std::cin >> is_sync;
    }

    double speed = 1;
    if (argc >= 4)
    {
        speed = atof(argv[3]);
    }
    else
    {
        printf("Input logging speed(times per second):");
        scanf("%lf", &speed);
    }

    auto logger = los::logs::CreateLogger(nullptr, kLogSize);
    LogThread(logger, is_sync, speed);
}

void TestMultiThreadLogging(int argc, char **argv)
{
    bool is_sync = false;
    if (argc >= 3)
    {
        is_sync = ('0' != argv[2][0]);
    }
    else
    {
        printf("Input is sync[0/1]:");
        std::cin >> is_sync;
    }

    int thread_cnt  = 1;
    if (argc >= 4)
    {
        thread_cnt = atoi(argv[3]);
    }
    else
    {
        printf("Input number of thread:");
        scanf("%d", &thread_cnt);
    }
    if (thread_cnt < 1) thread_cnt = 1;

    double speed = 1;
    if (argc >= 5)
    {
        speed = atof(argv[4]);
    }
    else
    {
        printf("Input logging speed(times per second):");
        scanf("%lf", &speed);
    }

    auto logger = los::logs::CreateLogger(nullptr, kLogSize);

    std::vector<std::thread> threads;
    for (int i = 0; i < thread_cnt; ++i)
    {
        threads.emplace_back(&LogThread, logger, is_sync, speed);
    }

    for (auto &&thd : threads)
    {
        if (thd.joinable())
        {
            thd.join();
        }
    }
}
