#include <iostream>

#include <string.h>
#include <unistd.h>

#include "imx8m.hh"

#include "config/config.hh"

#define ARR_SZ(x) (sizeof(x) / sizeof(*(x)))

using namespace std::literals;
using namespace std::chrono;

namespace imx8m {

    void Imx8m::__flush_all() noexcept {
        __main_logger.flush();
        for (size_t i = 0; i < __th_logger_pool.size(); ++i)
            __th_logger_pool[i].flush();
    }

    uint8_t Imx8m::checksum(const imx8m_packet *packet) noexcept {
        size_t i;
        uint8_t res;
        const uint8_t *tmp = reinterpret_cast<const uint8_t *>(packet);
        res = 0;
        ++tmp;
        for (i = 0; i < sizeof(*packet); ++i)
            res += tmp[i];
        res = ~res;
        return res;
    }

    void Imx8m::init(const char *path) noexcept {
        struct sched_param param;
        struct epoll_event ev;

        config::Config conf(path);

        if (!conf.is_ok()) {
            __error_str = conf.get_error_str();
            return;
        }

        __is_master = conf.is_master();

        for (size_t i = 0; i < conf.get_connections().size(); ++i) {
            __connections.push_back(network::UDPSocket_IpV4(conf.get_connections()[i].get_ip(),
                                    conf.get_connections()[i].get_port()));
        }

        /* INIT SELF SOCKET */
        __my_socket.init(conf.get_self_ip(), conf.get_self_port());
        if (!__my_socket.is_ok()) {
            __error_str = "imx8m: system error: socket";
            return;
        }

        [[gnu::unlikely]] if (!__my_socket.bind())
        {
            __error_str = "imx8m: system error: bind";
            return;
        }

        /* INIT EPOLL */

        if (__is_master)
            __do_stat_connections(1024);
        else {
            [[gnu::unlikely]] if ((__epoll_fd = epoll_create1(0)) == -1) {
                __error_str = "imx8m: system error: epoll_create1";
                return;
            }

            ev.data.fd = __my_socket;
            ev.events = EPOLLIN;
            if (epoll_ctl(__epoll_fd, EPOLL_CTL_ADD, __my_socket, &ev) == -1) {
                __error_str = "imx8m: system error: epoll_ctl";
                return;
            }
        }

        /* INIT THREAD */

        param.sched_priority = IMX8M_DEFAULT_THPRIO;
        [[gnu::unlikely]] if (pthread_setschedparam(pthread_self(),
                                                    SCHED_FIFO,
                                                    &param) != 0
                              || !__th_pool.set_sched(SCHED_FIFO,
                                                      IMX8M_DEFAULT_THPRIO + 1)
                              || !__th_timer.set_sched(SCHED_FIFO,
                                                       IMX8M_DEFAULT_THPRIO
                                                       + 5))
        {
            __error_str = "imx8m: system error: pthread_setschedparam";
            return;
        }
    }

    void Imx8m::__run_default() noexcept {
        int nfd;
        socklen_t socklen;

        imx8m_packet *packet;

        for (;;) {

            [[gnu::unlikely]] while ((nfd = epoll_wait(__epoll_fd,
                                                       __events,
                                                       ARR_SZ(__events),
                                                       -1)) == -1)
            {
                if (errno == EINTR)
                    continue;

                /* Exit because epoll_wait can only failed with bad param if errno != EINTR */

                std::cerr << "imx8m: system error: epoll_wait" << std::endl;
                __flush_all();
                std::terminate();
            }

            for (int i = 0; i < nfd; ++i) {
                packet = new(std::nothrow) imx8m_packet;

                [[gnu::unlikely]] if (packet == nullptr) {
                    __main_logger.push("imx8m: system error: out of memory");
                    continue;
                }

                [[gnu::unlikely]] if (!__my_socket.recv(packet,
                                                        sizeof(*packet),
                                                        0,
                                                        &socklen))
                {
                    __main_logger.push("imx8m: system error: recvfrom");
                    continue;
                }

                __th_pool.queue(&Imx8m::__do_recv, this, packet);
            }
        }
    }

    void Imx8m::__run_master() noexcept {
        /* No client */
        [[gnu::unlikely]] if (__timerstat.__max == nanoseconds(0))
        {
            std::cerr << "imx8m: no connection with any clients1" << std::endl;
            __flush_all();
            std::terminate();
        }

        imx8m_packet packet;
        socklen_t len;
        std::string line;

        for (;;) {
            std::getline(std::cin, line);
            for (size_t i = 0; i < __connections.size(); ++i) {
                bzero(&packet, sizeof(packet));

                packet.type = IMX8M_PICTURE_TIMER;
                packet.v_major = 1;
                packet.checksum = checksum(&packet);

                auto tmp = high_resolution_clock::now() + __timerstat.__max + 1000ns;
                packet.duration = duration_cast<nanoseconds>(tmp.time_since_epoch());

                [[gnu::unlikely]] if (__connections[i].send(&packet, sizeof(packet), 0)) {
                    __main_logger.push("imx8m: system error: sendto");
                    continue;
                }

                [[gnu::unlikely]] if (!__connections[i].recv(&packet,
                                                             sizeof(packet),
                                                             0,
                                                             &len))
                {
                    __main_logger.push("imx8m: system error: recvfrom");
                    continue;
                }

                [[gnu::unlikely]] if (!check_packet(packet)) {
                    __main_logger.push("imx8m: protocol error: check_packet failed");
                    continue;
                }

                switch (packet.type)
                {
                case IMX8M_ACK: break;
                case IMX8M_REJECTED:
                    __main_logger.push("imx8m: protocol error: packet rejected");
                    break;
                
                default:
                    __main_logger.push("imx8m: protocol error: packet type failed");
                    break;
                }
            }
        }
    }

    void Imx8m::__do_recv(const imx8m_packet *packet) noexcept {
        const size_t idx = __th_pool.get_last_idx();

        [[gnu::unlikely]] if (!check_packet(packet))
            __th_logger_pool[idx].push("imx8m: protocol error: check_packet failed");
        else {
            __th_timer.queue(TimePoint(nanoseconds(packet->duration)),
                             &Imx8m::__take_picture,
                             this);
        }
        delete packet;
    }

    [[gnu::cold]]
    void Imx8m::__do_stat_connections(const size_t& nb) noexcept {
        imx8m_packet packet;    
        socklen_t len;

        for (size_t cidx = 0; cidx < __connections.size(); ++cidx) {
            for (size_t i = 0; i < nb; ++i) {
                bzero(&packet, sizeof(packet));

                packet.v_major = 1;
                packet.type = IMX8M_TEST_CON;
                packet.checksum = checksum(&packet);

                auto start = high_resolution_clock::now();

                [[gnu::unlikely]] if (!__connections[cidx].send(&packet,
                                                                sizeof(packet),
                                                                0))
                {
                    __error_str = "imx8m: system error: sendto";
                    continue;
                }

                [[gnu::unlikely]] if (__connections[cidx].recv(&packet,
                                                               sizeof(packet),
                                                               0,
                                                               &len))
                {
                    __error_str = "imx8m: system error: recvfrom";
                    continue;
                }

                [[gnu::unlikely]] if (!check_packet(packet)) {
                    __error_str = "imx8m: protocol error: check_packet failed";
                    continue;
                }
                
                auto result = high_resolution_clock::now() - start;
                __timerstat.__max = std::max(__timerstat.__max, result);
                __timerstat.__min = std::min(__timerstat.__min, result);
            }
        }
        
    }
}