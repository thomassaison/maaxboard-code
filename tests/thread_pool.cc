#include <gtest/gtest.h>

#include "imx8m/thread/thread_pool.hh"

using namespace imx8m::thread;

namespace {
    TEST(ThreadPool, constructors) {
        ThreadPool t1;
        EXPECT_EQ(t1.size(), std::thread::hardware_concurrency() - 1);
        EXPECT_EQ(t1.get_current_thread_idx(), 0);

        ThreadPool t2(std::move(t1));
        EXPECT_EQ(t2.size(), std::thread::hardware_concurrency() - 1);
        EXPECT_EQ(t2.get_current_thread_idx(), 0);

        ThreadPool t3(5);
        EXPECT_EQ(t3.size(), 5);
        EXPECT_EQ(t3.get_current_thread_idx(), 0);
    }

    TEST(ThreadPool, swap) {
        ThreadPool t1(2), t2(3);
        EXPECT_EQ(t1.size(), 2);
        EXPECT_EQ(t2.size(), 3);
        EXPECT_EQ(t1.get_current_thread_idx(), 0);
        EXPECT_EQ(t2.get_current_thread_idx(), 0);

        t1.swap(t2);
        EXPECT_EQ(t1.size(), 3);
        EXPECT_EQ(t2.size(), 2);
        EXPECT_EQ(t1.get_current_thread_idx(), 0);
        EXPECT_EQ(t2.get_current_thread_idx(), 0);
    }

    TEST(ThreadPool, operator_eq) {
        ThreadPool t1(2), t2(3);
        EXPECT_EQ(t1.size(), 2);
        EXPECT_EQ(t2.size(), 3);
        EXPECT_EQ(t1.get_current_thread_idx(), 0);
        EXPECT_EQ(t2.get_current_thread_idx(), 0);

        t1 = std::move(t2);
        EXPECT_EQ(t1.size(), 3);
        EXPECT_EQ(t1.get_current_thread_idx(), 0);
    }

    TEST(ThreadPool, set_sched) {
        ThreadPool t;
        EXPECT_EQ(t.size(), std::thread::hardware_concurrency() - 1);
        EXPECT_EQ(t.get_current_thread_idx(), 0);
        t.set_sched(SCHED_FIFO, 10);
    }

    TEST(ThreadPool, queue) {
        ThreadPool t(5);
        EXPECT_EQ(t.size(), 5);
        EXPECT_EQ(t.get_current_thread_idx(), 0);
        for (size_t x = 0; x < 4; ++x) {
            t.queue([&](int i){ std::cerr << "From thread " << i << std::endl; }, x);
            EXPECT_EQ(t.get_current_thread_idx(), x + 1);
        }

        t.queue([&](int i){ std::cerr << "From thread " << i << std::endl; }, 4);
        EXPECT_EQ(t.get_current_thread_idx(), 0);

        sleep(2);

        std::cout << "From Main thread" << std::endl;
    }
}