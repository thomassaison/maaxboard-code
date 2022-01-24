#pragma once

#include <chrono>
#include <algorithm>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "thread/timer_thread.hh"
#include "thread/thread_pool.hh"
#include "network/udp_socket_ipv4.hh"
#include "logger/logger.hh"

#define IMX8M_DEFAULT_THPRIO 50

namespace imx8m {

    struct imx8m_packet {
        uint8_t checksum;

        uint8_t v_major : 4,
                v_minor : 4;

#define IMX8M_ACK           1
#define IMX8M_TEST_CON      2
#define IMX8M_PICTURE_TIMER 3
#define IMX8M_REJECTED      4
        uint8_t type;

        uint8_t  reserved1;
        uint32_t reserved2;

        int64_t duration;
                
    } __attribute__((__packed__));

    class Imx8m
    {
    public:
        using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

        Imx8m() noexcept
            : __error_str{"imx8m: no init"},
              __th_pool{std::thread::hardware_concurrency() - 2}
        {}

        Imx8m(const char *path) noexcept
            : __th_pool{std::thread::hardware_concurrency() - 2}
        {
            this->init(path);
        }

        Imx8m(std::string& path) noexcept
            : Imx8m(path.c_str())
        {}

        [[gnu::cold]]
        void init(const char *path) noexcept;

        [[gnu::cold]]
        void init(std::string& path) noexcept {
            this->init(path.c_str());
        }

        ~Imx8m() noexcept;

        [[noreturn]] void run() noexcept {
            (__is_master) ? __run_master() : __run_default();
        }

        bool is_master() noexcept {
            return __is_master;
        }

        bool is_ok() noexcept {
            return !__error_str.empty();
        }

        static void assert_abi() noexcept {
            static_assert(sizeof(long) == 8);
            static_assert(sizeof(Imx8m) > 10);
        }

    private:
        struct Timerstat {
            Timerstat()
                : __max{0}, __min{0}
            {}

            std::chrono::nanoseconds __max;
            std::chrono::nanoseconds __min;
        };
        
        int                                  __epoll_fd;
        epoll_event                          __events[10];
        network::UDPSocket_IpV4              __my_socket;
        bool                                 __is_master;
        std::string                          __error_str;
        std::vector<network::UDPSocket_IpV4> __connections;
        logger::Logger                       __main_logger;
        std::vector<logger::Logger>          __th_logger_pool;
        thread::ThreadPool                   __th_pool;
        thread::TimerThread                  __th_timer;

        Timerstat                            __timerstat;
    
        [[noreturn]] void __run_default() noexcept;
        [[noreturn]] void __run_master() noexcept;

        static uint8_t checksum(const imx8m_packet *packet) noexcept;

        static bool check_packet(const imx8m_packet *packet) {
            return packet->v_major == 1
                && packet->v_minor == 0
                && packet->checksum == checksum(packet);
        }

        static bool check_packet(const imx8m_packet& packet) {
            return check_packet(&packet);
        }

        void __do_recv(const imx8m_packet *packet) noexcept;

        [[gnu::cold]]
        void __do_stat_connections(const size_t& nb) noexcept;

        void __take_picture() noexcept {

        }
    };

    inline long roundMax(long a, long b) {
        return (a + (b - 1)) / b;
    }
};