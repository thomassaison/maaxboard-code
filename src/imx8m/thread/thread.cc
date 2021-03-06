#include "thread.hh"

namespace imx8m {
    namespace thread {

        Thread::~Thread() noexcept {
            __stop = true;
            __cond.notify_all();
            __th.join();
        };

        bool Thread::in_work() noexcept {
            std::unique_lock<std::mutex> lock(__mutex);
            return !__jobs.empty();
        }

        bool Thread::set_sched(const int policy, const int priority) noexcept {
            sched_param param;
            param.sched_priority = priority;
            return pthread_setschedparam(__th.native_handle(), policy, &param) == 0;
        }

        void Thread::swap(Thread& t) noexcept {
            std::unique_lock<std::mutex> lock(__mutex);
            __jobs.swap(t.__jobs);
        }

        void Thread::__run() noexcept {
            std::packaged_task<void()> task;

            for (;;) {

                {
                    std::unique_lock<std::mutex> lock(__mutex);
                    __cond.wait(lock, [&] { return !__jobs.empty() || __stop; });
                    if (__stop)
                        return;

                    task.swap(__jobs.front());
                    __jobs.pop();
                }
                task();
            }
        }

        void Thread::__add_task(std::packaged_task<void()>&& task) noexcept {
            {
                std::unique_lock<std::mutex> lock(__mutex);
                __jobs.emplace(std::move(task)); /* Store task<R> as task<void> */
            }

            __cond.notify_one();
        }
    };
};

