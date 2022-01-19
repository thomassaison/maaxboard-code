#include "timer_thread.hh"

namespace imx8m {
    namespace thread {
        bool TimerThread::set_sched(const int policy, const int priority) noexcept {
            sched_param param;
            param.sched_priority = priority;
            return pthread_setschedparam(__th.native_handle(), policy, &param) == 0;
        }

        void TimerThread::__run() noexcept {
            std::packaged_task<void()> task;
            std::chrono::milliseconds  tmp;
            for (;;) {
                {
                    std::unique_lock<std::mutex> lock(__mutex);
                    __cond.wait(lock, [&] { return !__timer_queue.empty(); });
                    tmp = __timer_queue.front();
                    task.swap(__task_queue.front());
                    __timer_queue.pop();
                    __task_queue.pop();
                }
                std::this_thread::sleep_for(tmp);
                task();
            }
        }

        void TimerThread::swap(TimerThread& t) noexcept {
            std::unique_lock<std::mutex> lock(__mutex);
            __timer_queue.swap(t.__timer_queue);
            __task_queue.swap(t.__task_queue);
        }
    }
}