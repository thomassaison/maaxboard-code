#include "thread_pool.hh"

namespace imx8m {
    namespace thread {
        ThreadPool::ThreadPool(const unsigned int nb) noexcept
        {
            __th_first_idx = 0;
            __th_last_idx  = nb - 1;
            for (unsigned int i = 0; i < nb; ++i)
                __th_queue.push_back(Thread());
        }


        bool ThreadPool::set_sched(const int policy, const int priority) noexcept
        {
            bool res = true;
            for (size_t i = 0; i < __th_queue.size() && res; ++i)
                res = __th_queue[i].set_sched(policy, priority);
            return res;
        }


        void ThreadPool::swap(ThreadPool& th) noexcept {
            __th_queue.swap(th.__th_queue);
            std::swap(__th_first_idx, th.__th_first_idx);
            std::swap(__th_last_idx, th.__th_last_idx);
        }

        void ThreadPool::__update_idx() noexcept {
            __th_first_idx = (__th_first_idx + 1) % __th_queue.size();
            __th_last_idx = (__th_first_idx + 1) % __th_queue.size();
        }
    }
}