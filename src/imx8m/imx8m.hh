#pragma once

#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include  <chrono>

#include "thread/timer_thread.hh"
#include "thread/thread_pool.hh"
#include "thread/timerstat.hh"

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

    struct timerstat {
        long                __max  = 0;
        long                __min = 0;
    };

    class Imx8m
    {
    public:
        using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

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

    private:
        int                 __epoll_fd;
        epoll_event         __events[10];
        int                 __my_socket;
        bool                __is_master;
        bool                __error;
        sockaddr_in         __sa;
        thread::ThreadPool  __th_pool;
        thread::TimerThread __th_timer;
        struct timerstat    __timerstat;

        [[noreturn]] void __run_default() noexcept;

        [[noreturn]] void __run_master() noexcept;

        static bool checksum(const imx8m_packet *packet) {
            size_t i, res = 0;
            const uint8_t *tmp = reinterpret_cast<const uint8_t *>(packet);
            for (i = 0; i < sizeof(*packet); ++i)
                res += tmp[i];
            res = ~res;
            return res;
        }

        static bool check_packet(const imx8m_packet *packet) {
            return packet->v_major == 1
                && packet->v_minor == 0
                && packet->checksum == checksum(packet);
        }

        static bool check_packet(const imx8m_packet& packet) {
            return check_packet(&packet);
        }

        void __mean(long &v) noexcept{
            __mean += (v + __timer1) / 2;
        }

        void __do_recv(const imx8m_packet *packet) noexcept {

            [[gnu::likely]] if (check_packet(packet)) {
                std::chrono::nanoseconds dur(packet->duration);
                __th_timer.queue(TimePoint(dur), &Imx8m::__take_picture, this);
            }

            delete packet;
        }


        void __do_stat(const size_t &nb) noexcept {
            timerstat stat;
            imx8m_packet packet;
            for (size_t i = 0; i < nb; ++i){
                bzero(&packet, sizeof(packet));
                packet.v_major = 1;
                packet.type = IMX8M_TEST_CON;
                socklen_t len = 0; //TODO
            //TODO CHECKSUM
                auto timer1 = std::chrono::high_resolution_clock::now();
                [[gnu::unlikely]] while ((ret = sendto(__my_socket,
                                                         &packet,
                                                         sizeof(packet),
                                                         0 /* flags */,
                                                         reinterpret_cast<sockaddr *>(&__sa) /* todo */,
                                                         &len) == -1))
                {
                    if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
                        std::cerr << "imx8m: sendto error" << std::endl;
                        exit(EXIT_FAILURE);
                    }
                }

                [[gnu::unlikely]] while ((ret = recvfrom(__my_socket,
                                                         &packet,
                                                         sizeof(packet),
                                                         0 /* flags */,
                                                         reinterpret_cast<sockaddr *>(&__sa),
                                                         &len) == -1))
                {
                    if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
                        std::cerr << "imx8: recvfrom error" << std::endl;
                        exit(EXIT_FAILURE);
                    }
                }

                // TODO check packet

                auto result = __subs_timer(timer1);
                stat.__max = std::max(stat.__max, result);
                stat.__min = std::min(stat.__min, result);
            }
        }

        std::chrono::nanoseconds __subs_timer(std::chrono::nanoseconds &t) noexcept {
            return result(t - std::chrono::high_resolution_clock::now());
        }

        void __recv_test_con() noexcept {

        }

        void __take_picture() noexcept {

        }
    };

    inline long __roundMax(long a, long b){
        return (a + (b - 1)) / b;
    }
};