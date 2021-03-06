#pragma once

#include "thread.hh"

namespace imx8m {
    namespace thread {
        class ThreadPool
        {
        public:
            ThreadPool(const unsigned int nb) noexcept;

            ThreadPool() noexcept
                : ThreadPool(std::thread::hardware_concurrency() - 1) 
            {}

            ThreadPool(const ThreadPool&) = delete;

            ThreadPool(ThreadPool&& th) noexcept
                : ThreadPool()
            {
                this->swap(th);
            }

            ThreadPool& operator=(const ThreadPool&) = delete;

            ThreadPool& operator=(ThreadPool&& th) noexcept {
                this->swap(th);
                return *this;
            }

            ~ThreadPool() = default;

            bool set_sched(const int policy, const int priority) noexcept;

            template <class F, class... Args, class R = std::result_of_t<F&(Args...)>>
            std::future<R> queue(F&& f, Args&&... args) noexcept {
                /* Thread::queue is thread safe and vector is const at this point so no mutex needed */
                auto ret = __th_queue[__th_first_idx].queue(std::forward<F>(f), std::forward<Args>(args)...);
                __update_idx();
                return ret;
            }

            size_t get_last_idx() const noexcept {
                return (__th_first_idx == 0) ? __th_queue.size() - 1 : __th_first_idx - 1;
            }

            void swap(ThreadPool& th) noexcept;

            size_t get_current_thread_idx() const noexcept {
                return __th_first_idx;
            }

            size_t size() noexcept {
                std::unique_lock<std::mutex> lock(__mutex);
                return __th_queue.size();
            }

            const Thread& get_current_thread() const noexcept {
                return __th_queue[__th_first_idx];
            }

        private:
            std::mutex          __mutex;
            std::vector<Thread> __th_queue;
            std::atomic<size_t> __th_first_idx;

            void __update_idx() noexcept;
        };
    };
};
