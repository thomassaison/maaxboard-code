#pragma once

#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "thread/timer_thread.hh"
#include "thread/thread_pool.hh"

#define IMX8M_DEFAULT_THPRIO 50

namespace imx8m {
    struct imx8m_slave_request {
        uint8_t v_major : 4,
                v_minor : 4;

        
                
    } __attribute__((__packed__));

    class Imx8m
    {
    public:
        Imx8m(const char *path) noexcept;

        Imx8m(std::string& path) noexcept
            : Imx8m(path.c_str())
        {}

        ~Imx8m() = default; // TODO

        void run() noexcept {
            (__is_master) ? __run_master() : __run_default();
        }


        bool is_master() noexcept {
            return __is_master;
        }

        bool is_ok() noexcept {
            return !__error;
        }


    private:
        int                __epoll_fd;
        epoll_event        __events[10];
        int                __my_socket;
        bool               __is_master;
        bool               __error;
        sockaddr_in        __sa;
        thread::ThreadPool __th_pool;

        [[noreturn]] void __run_default() noexcept;
        void __run_master() noexcept;

        void __do_nothing() noexcept {}

    };

    extern thread::TimerThread timer_th;
    
};