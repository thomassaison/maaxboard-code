#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <thread>
#include <functional>

#include <algorithm>

#include <pthread.h>

namespace imx8m {
    namespace thread {
        class Thread
        {
        public:
            Thread() noexcept
                : __th{std::thread(&Thread::__run, this)}
            {}

            ~Thread() = default;

            Thread(const Thread&) = delete;
            Thread& operator=(const Thread&) = delete;

            Thread(Thread&& th) noexcept
                : Thread()
            {
                this->swap(th);
            }

            Thread& operator=(Thread&& th) noexcept {
                this->swap(th);
                return *this;
            }

            auto get_id() const noexcept {
                return __th.get_id();
            }

            bool set_sched(const int policy, const int priority) noexcept;

            bool in_work() noexcept;

            template <class F, class ...Args, class R = std::result_of_t<F&(Args...)>>
            std::future<R> queue(F&& f, Args&&... args) {
                std::packaged_task<R()> task(std::bind(f, std::forward<Args>(args)...));
                std::future<R> res = task.get_future();
                __add_task(std::move(task));
                return res;
            }

            void swap(Thread& t) noexcept;

        private:
            std::thread                            __th;
            std::condition_variable                __cond;
            std::queue<std::packaged_task<void()>> __jobs;
            std::mutex                             __jobs_mutex;

            void __add_task(std::packaged_task<void()>&& task) noexcept;
            [[noreturn]] void __run() noexcept;
        };
    };
};
