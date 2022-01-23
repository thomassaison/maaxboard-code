#pragma once

#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include  <chrono>

#include "thread/timer_thread.hh"
#include "thread/thread_pool.hh"

#define IMX8M_DEFAULT_THPRIO 50

namespace imx8m {

    struct imx8m_packet {
        uint8_t v_major : 4,
                v_minor : 4;

#define IMX8M_ACK           1
#define IMX8M_TEST_CON      2
#define IMX8M_PICTURE_TIMER 3
        uint8_t type;
        uint8_t checksum;

        uint8_t  reserved1;
        uint32_t reserved2;

        int64_t duration;
                
    } __attribute__((__packed__));

    class Imx8m
    {
    public:
        Imx8m(const char *path) noexcept;

        Imx8m(std::string& path) noexcept
            : Imx8m(path.c_str())
        {}

        ~Imx8m() = default; // TODO

        [[noreturn]] void run() noexcept {
            (__is_master) ? __run_master() : __run_default();
        }


        bool is_master() noexcept {
            return __is_master;
        }

        bool is_ok() noexcept {
            return !__error;
        }

        static void assert_abi() noexcept {
            static_assert(sizeof(long) == 8);
            static_assert(sizeof(Imx8m) > 10);
        }

        static bool check_packet(const imx8m_packet *packet) {
            size_t sum, i;
            const uint8_t *tmp = reinterpret_cast<const uint8_t *>(packet);

            if (packet->v_major != 1 || packet->v_minor != 0)
                return false;

            for (sum = i = 0; i < sizeof(*packet); ++i)
                sum += tmp[i];
            sum = ~sum;

            return sum == packet->checksum;
        }

        static bool check_packet(const imx8m_packet& packet) {
            return check_packet(&packet);
        }


    private:
        int                 __epoll_fd;
        epoll_event         __events[10];
        int                 __my_socket;
        bool                __is_master;
        bool                __error;
        sockaddr_in         __sa;
        thread::ThreadPool  __th_pool;
        thread::TimerThread __th_timer;

        [[noreturn]] void __run_default() noexcept;

        [[noreturn]] void __run_master() noexcept;


        void __do_recv(const imx8m_packet *packet) noexcept {

            if (!check_packet(packet))
                return;

            std::chrono::nanoseconds dur(packet->duration);
            __th_timer.queue_task_at(std::chrono::time_point<std::chrono::high_resolution_clock>(dur),
                             &Imx8m::__take_picture,
                             this);

            delete packet;
        }

        void __recv_test_con() noexcept {

        }

        void __take_picture() noexcept {

        }

    };    
};