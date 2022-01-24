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

        ~Imx8m() noexcept
        {
            this->__flush_all();
        }

        bool in(const sockaddr_in *addr) const noexcept {
            for (auto elm: __connections_sa) {
                if (elm.sin_addr.s_addr == addr->sin_addr.s_addr
                    && elm.sin_port == addr->sin_port
                    && elm.sin_family == AF_INET)
                    return true;
            }
            return false;
        }

        bool in(const sockaddr *addr, const socklen_t len) const noexcept {
            for (auto elm: __connections_sa) {
                if (memcmp(addr, &elm, len) == 0)
                    return true;
            }
            return false;
        }

        [[noreturn]] void run() noexcept {
            (__is_master) ? __run_master() : __run_default();
        }

        bool is_master() noexcept {
            return __is_master;
        }

        bool is_ok() noexcept {
            return !__error_str.empty();
        }

        const std::string& get_error_str() const noexcept {
            return __error_str;
        }

        static void assert_abi() noexcept {
            static_assert(sizeof(long) == 8);
            static_assert(sizeof(Imx8m) > 10);
        }

    private:
        struct imx8m_packet {

            uint8_t v_major : 4,
                    v_minor : 4;

#define IMX8M_ACK           1
#define IMX8M_TEST_CON      2
#define IMX8M_PICTURE_TIMER 3
#define IMX8M_REJECTED      4
            uint8_t type;

            uint16_t checksum;

            uint32_t reserved2;

            int64_t duration;
                
        } __attribute__((__packed__));

        struct Timerstat {
            Timerstat()
                : __max{0}, __min{0}
            {}

            std::chrono::nanoseconds __max;
            std::chrono::nanoseconds __min;
        };

        int                                  __epoll_fd; // OK
        epoll_event                          __events[10]; // OK
        sockaddr_in                          __sa; // OK
        std::vector<sockaddr_in>             __connections_sa; // TODO
        network::UDPSocket_IpV4              __my_socket; // OK
        bool                                 __is_master; // OK
        std::string                          __error_str; // OK
        std::vector<network::UDPSocket_IpV4> __th_sockets; // OK
        logger::Logger                       __main_logger; // OK
        std::vector<logger::Logger>          __th_logger_pool; // OK
        thread::ThreadPool                   __th_pool; // OK
        thread::TimerThread                  __th_timer; // OK

        Timerstat                            __timerstat;
    
        [[noreturn]] void __run_default() noexcept;
        [[noreturn]] void __run_master() noexcept;

        void __flush_all() noexcept;

        static uint16_t checksum(const imx8m_packet *packet) noexcept;

        static bool check_packet(const imx8m_packet *packet) {
//#ifndef NDEBUG
            if (packet->v_major != 1)
                std::cout << "check_packet: Bad majeur" << std::endl;
            if (packet->v_minor != 0)
                std::cout << "check_packet: Bad minor" << std::endl;
            /*if (packet->checksum != checksum(packet))
                std::cout << "check_packet: Bad checksum" << std::endl;*/
//#endif /* NDEBUG */
            return packet->v_major == 1
                && packet->v_minor == 0;
               // && packet->checksum == checksum(packet);
        }

        static bool check_packet(const imx8m_packet& packet) {
            return check_packet(&packet);
        }

        void __do_recv(const imx8m_packet *packet,
                       const sockaddr *addr,
                       const socklen_t len) noexcept;

        [[gnu::cold]]
        void __do_stat_connections(const size_t& nb) noexcept;

        void __take_picture() noexcept {
            std::cout << "picture taken\n";
        }
    };
};