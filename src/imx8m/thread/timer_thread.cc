#include "timer_thread.hh"

namespace imx8m {
    namespace thread {
        void TimerThread::__add_task_at(TimerThreadTask&& task) noexcept
        {
            {
                std::unique_lock<std::mutex> lock(__mutex);
                __jobs_queue.insert(task);
            }

            __cond.notify_one();
        }

        bool TimerThread::set_sched(const int policy, const int priority) noexcept {
            sched_param param;
            param.sched_priority = priority;
            return pthread_setschedparam(__th.native_handle(), policy, &param) == 0;
        }

        void TimerThread::__run() noexcept {
            TimerThreadTask task;

            for (;;) {
                {
                    std::unique_lock<std::mutex> lock(__mutex);
                    __cond.wait(lock, [&] { return !__jobs_queue.empty(); });

                    task.swap(__jobs_queue.front());
                    __jobs_queue.pop();
                }

                auto now = std::chrono::high_resolution_clock::now();
                if (task > now) // In fact this is probably just an integer comparison, so it don't take time
                    std::this_thread::sleep_for(now - task.get_time_point());
                task();
            }
        }

        void TimerThread::swap(TimerThread& t) noexcept {
            std::unique_lock<std::mutex> lock(__mutex);
            __jobs_queue.swap(t.__jobs_queue);
        }
    }
}