#include "timer_thread.hh"

namespace imx8m {
    namespace thread {

        TimerThread::~TimerThread() noexcept {
            __stop = true;
            __cond.notify_all();
            __th.join();
        };

        void TimerThread::__add_task_at(Task&& task) noexcept
        {
            {
                std::unique_lock<std::mutex> lock(__mutex);
                __jobs.emplace(std::move(task));
            }

            __cond.notify_one();
        }

        bool TimerThread::set_sched(const int policy, const int priority) noexcept {
            sched_param param;
            param.sched_priority = priority;
            return pthread_setschedparam(__th.native_handle(), policy, &param) == 0;
        }

        void TimerThread::__run() noexcept {
            Task task;

            for (;;) {
                {
                    std::unique_lock<std::mutex> lock(__mutex);
                    __cond.wait(lock, [&] { return !__jobs.empty() || __stop; });
                    if (__stop)
                        return;

                    task.swap(__jobs.front());
                    __jobs.pop();
                }
                //std::this_thread::sleep_until(task.get_time_point());
                std::this_thread::sleep_for(task.get_time_point() - std::chrono::high_resolution_clock::now());
                task();
            }
        }

        void TimerThread::swap(TimerThread& t) noexcept {
            std::unique_lock<std::mutex> lock(__mutex);
            __jobs.swap(t.__jobs);
        }
    }
}