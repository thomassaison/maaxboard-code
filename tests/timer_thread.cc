#include <gtest/gtest.h>

#include <chrono>

#include "imx8m/thread/timer_thread.hh"

using namespace imx8m::thread;
using namespace std::literals;
using namespace std::chrono;

namespace {
    TEST(TimerThread, constructors) {
        TimerThread t1;
        EXPECT_FALSE(t1.in_work());

        TimerThread t2(std::move(t1));
        EXPECT_FALSE(t2.in_work());
    }

    TEST(TimerThread, operator_eq) {
        TimerThread t1, t2;
        EXPECT_FALSE(t1.in_work());
        EXPECT_FALSE(t2.in_work());

        t1 = std::move(t2);
    }

    TEST(TimerThread, set_sched) {
        TimerThread t;
        t.set_sched(SCHED_FIFO, 10);
    }

    TEST(TimerThread, queue) {
        TimerThread t;

        auto start = high_resolution_clock::now();

        t.queue(start + 100000ns, [&](auto tmp) {
            std::cerr << "From thread "
                      << duration_cast<nanoseconds>(high_resolution_clock::now() - tmp).count()
                      << std::endl;
        }, start);

        sleep(1);
    }
}