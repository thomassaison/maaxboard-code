#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <future>
#include <functional>
#include <chrono>
#include <condition_variable>
#include <optional>

#include <imx8m/container/ring_vector.hh>

namespace imx8m {
    namespace thread {
        class TimerThread
        {
        public:
            using Clock = std::chrono::time_point<std::chrono::high_resolution_clock>;

            TimerThread() noexcept
                : __th{std::thread(&TimerThread::__run, this)},
                  __stop{false}
            {}

            ~TimerThread() noexcept;

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
            
            template <class F, class ...Args, class R = std::result_of_t<F&(Args...)>>
            std::future<R> queue(const Clock& time, F&& f, Args&&... args)
            {
                std::packaged_task<R()> task(std::bind(f, std::forward<Args>(args)...));
                std::future<R> res(task.get_future());
                Task t(std::move(task), time);
                __add_task_at(std::move(t));
                return res;
            }

            static void compiletime_assert() noexcept {
                /*  
                    If not you  should probably change the implementation because
                    insert on vector maye be slow)
                */
                static_assert(sizeof(std::chrono::nanoseconds) < 32);
                static_assert(sizeof(std::packaged_task<void()>) < 32);
            }

            bool can_insert(const Clock& time) {
                std::unique_lock<std::mutex> l(__mutex);
                return __jobs.back() <= time;
            }

            bool in_work() noexcept {
                std::unique_lock<std::mutex> l(__mutex);
                return !__jobs.empty();
            }
            
        private:

            class Task
            {
            public:
                Task() = default;

                Task(std::packaged_task<void()>&& task, const Clock& time) noexcept
                    : __at{time}
                {
                    __task.swap(task);
                }

                Task(const Task&) = delete;

                Task(Task&& task) noexcept
                {
                    this->swap(task);
                }

                Task& operator=(const Task&) = delete;

                Task& operator=(Task&& task) noexcept
                {
                    this->swap(task);
                    return *this;
                }

                ~Task() = default;

                bool operator<(const Clock& c) const noexcept {
                    return c < __at;
                }

                bool operator<(const Task& t) const noexcept {
                    return t < __at;
                }

                bool operator<=(const Clock& c) const noexcept {
                    return c <= __at;
                }

                bool operator<=(const Task& t) const noexcept {
                    return t <= __at;
                }

                bool operator>(const Clock& c) const noexcept {
                    return c > __at;
                }

                bool operator>(const Task& t) const noexcept {
                    return t > __at;
                }

                bool operator>=(const Clock& c) const noexcept {
                    return c >= __at;
                }

                bool operator>=(const Task& t) const noexcept {
                    return t >= __at;
                }

                bool operator==(const Clock& c) const noexcept {
                    return c == __at;
                }

                bool operator==(const Task& t) const noexcept {
                    return t == __at;
                }

                bool operator!=(const Clock& c) const noexcept {
                    return c == __at;
                }

                bool operator!=(const Task& t) const noexcept {
                    return t != __at;
                }

                void operator()() noexcept {
                    __task();
                }

                void swap(Task& task) noexcept {
                    __task.swap(    task.__task);
                    std::swap(__at, task.__at);
                }

                const Clock& get_time_point() const noexcept {
                    return __at;
                }

            private:
                Clock                      __at;
                std::packaged_task<void()> __task;
            };

            std::thread             __th;
            std::mutex              __mutex;
            std::condition_variable __cond;
            std::queue<Task>        __jobs;
            std::atomic<bool>       __stop;

            /*container::UniqueRingVectorSorted<TimerThreadTask> __jobs_queue;*/

            void __add_task_at(Task&& task) noexcept;
            void __run() noexcept;
        };
    }
}