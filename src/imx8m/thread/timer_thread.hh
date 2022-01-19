#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <future>

#include <condition_variable>

namespace imx8m {
    namespace thread {
        class TimerThread
        {
        public:
            TimerThread() noexcept
                : __th{std::thread(&TimerThread::__run, this)}
            {}

            ~TimerThread()                             = default;

            TimerThread(const TimerThread&)            = delete;
            TimerThread& operator=(const TimerThread&) = delete;

            TimerThread(TimerThread&& th) noexcept
                : TimerThread()
            {
                this->swap(th);
            }

            TimerThread& operator=(TimerThread&& th) noexcept {
                this->swap(th);
                return *this;
            }

            void swap(TimerThread& t) noexcept;
            bool set_sched(const int policy, const int priority) noexcept;
            
        private:
            std::thread                            __th;
            std::mutex                             __mutex;
            std::condition_variable                __cond;
            std::queue<std::chrono::milliseconds>  __timer_queue;
            std::queue<std::packaged_task<void()>> __task_queue;

            [[noreturn]] void __run() noexcept;
        };
    }
}