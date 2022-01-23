#include <gtest/gtest.h>

#include "imx8m/thread/thread.hh"

using namespace imx8m::thread;

namespace {
    TEST(Thread, constructors) {
        Thread t1;
        EXPECT_FALSE(t1.in_work());

        Thread t2(std::move(t1));
        EXPECT_FALSE(t2.in_work());
    }

    TEST(Thread, operator_eq) {
        Thread t1, t2;
        EXPECT_FALSE(t1.in_work());
        EXPECT_FALSE(t2.in_work());

        t1 = std::move(t2);
    }

    TEST(Thread, set_sched) {
        Thread t;
        t.set_sched(SCHED_FIFO, 10);
    }

    TEST(Thread, queue) {
        Thread t;
        for (size_t x = 0; x < 10; ++x)
            t.queue([&](int i){ std::cerr << "From thread " << i << std::endl; }, x);
        sleep(1);
        std::cout << "From Main thread" << std::endl;
    }
}