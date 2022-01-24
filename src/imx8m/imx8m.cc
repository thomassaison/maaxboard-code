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

    uint16_t Imx8m::checksum(const imx8m_packet *packet) noexcept {
        unsigned long res = 0;
        const uint16_t *tmp = reinterpret_cast<const uint16_t *>(packet);
        size_t size = sizeof(*packet);

        while(size > 1) {
            res += *tmp++;
            size -= sizeof(uint16_t);
        }
        res = (res >> 16) + (res & 0xffff);
        res += (res >> 16);
        return static_cast<uint16_t>(~res);
    }

    void Imx8m::init(const char *path) noexcept {
        sched_param param;
        epoll_event ev;
        std::string tmp;

        [[gnu::unlikely]] if (!__my_socket.is_ok()) {
            __error_str = "imx8m: system error: socket";
            return;
        }

        config::Config conf(path);

        if (!conf.is_ok()) {
            __error_str = conf.get_error_str();
            return;
        }
        __is_master = conf.is_master();
        
        for (size_t i = 0; i < __th_pool.size(); ++i) {
            __th_sockets.push_back(network::UDPSocket_IpV4());
            __th_logger_pool.push_back(logger::Logger("thread" + std::to_string(i) + ".log"));
            [[gnu::unlikely]] if (!__th_logger_pool[i].is_ok()) {
                __error_str = "imx8m: thread logger failed";
                return;
            }
        }

        __main_logger.open("main.log");
        [[gnu::unlikely]] if (!__main_logger.is_ok()) {
            __error_str = "imx8m: main logger failed";
            return;
        }

        for (size_t i = 0; i < conf.get_connections().size(); ++i) {
            bzero(&__sa, sizeof(__sa));
            __sa.sin_family = AF_INET;
            __sa.sin_port = htons(conf.get_connections()[i].get_port());
            [[gnu::unlikely]] if (inet_pton(AF_INET, conf.get_connections()[i].get_ip().c_str(), &__sa.sin_addr) != 1) {
                __error_str = "imx8m: netword: bad ip format";
                return;
            }

            __connections_sa.push_back(__sa);
        }

        bzero(&__sa, sizeof(__sa));
        __sa.sin_family = AF_INET;
        __sa.sin_port = htons(conf.get_self_port());
        if (inet_pton(AF_INET, conf.get_self_ip().c_str(), &__sa.sin_addr) != 1) {
            __error_str = "imx8m: netword: bad ip format";
            return;
        }

        [[gnu::unlikely]] if (!__my_socket.bind(reinterpret_cast<sockaddr *>(&__sa), sizeof(__sa)))
        {
            __error_str = "imx8m: system error: bind";
            return;
        }

        /* INIT EPOLL */

        if (__is_master)
            (void)0;//__do_stat_connections(10);
        else {
            [[gnu::unlikely]] if ((__epoll_fd = epoll_create1(0)) == -1) {
                __error_str = "imx8m: system error: epoll_create1";
                return;
            }

            ev.data.fd = __my_socket;
            ev.events = EPOLLIN;
            [[gnu::unlikely]] if (epoll_ctl(__epoll_fd, EPOLL_CTL_ADD, __my_socket, &ev) == -1) {
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

        std::cout << "test1" << std::endl;
    }

    void Imx8m::__run_default() noexcept {
        int nfd;
        socklen_t socklen;
        imx8m_packet *packet;
        sockaddr *addr;

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
                addr = new(std::nothrow) sockaddr;

                [[gnu::unlikely]] if (packet == nullptr || addr == nullptr) {
                    __main_logger.push("imx8m: system error: out of memory");
                    continue;
                }

                [[gnu::unlikely]] if (!__my_socket.recv(packet,
                                                        sizeof(*packet),
                                                        0,
                                                        addr,
                                                        &socklen))
                {
                    __main_logger.push("imx8m: system error: recvfrom");
                    continue;
                }

                __th_pool.queue(&Imx8m::__do_recv, this, packet, addr, socklen);
            }
        }
    }

    void Imx8m::__run_master() noexcept {
        imx8m_packet packet;
        socklen_t len;
        std::string line;
        sockaddr addr;

        for (;;) {
            getchar();

            std::cin.clear();

            std::cout << "line read\n"
                      << "con size: "
                      << __connections_sa.size()
                      << std::endl;
            
            for (size_t i = 0; i < __connections_sa.size(); ++i) {
                bzero(&packet, sizeof(packet));

                packet.type = IMX8M_PICTURE_TIMER;
                packet.v_major = 1;
            
                auto tmp = high_resolution_clock::now() + __timerstat.__max + 1000ns;
                packet.duration = duration_cast<nanoseconds>(tmp.time_since_epoch()).count();
                packet.checksum = checksum(&packet);

                [[gnu::unlikely]] if (!__th_sockets[0].send(&packet,
                                                            sizeof(packet),
                                                            0,
                                                            reinterpret_cast<sockaddr *>(&__connections_sa[i]),
                                                            sizeof(sockaddr_in)))
                {    
                    __main_logger.push("imx8m: system error: sendto");
                    std::cout << "failed1" << std::endl;
                    continue;
                }

                [[gnu::unlikely]] if (!__th_sockets[0].recv(&packet,
                                                            sizeof(packet),
                                                            0,
                                                            &addr,
                                                            &len))
                {
                    __main_logger.push("imx8m: system error: recvfrom");
                    std::cout << "failed2" << std::endl;
                    continue;
                }

                [[gnu::unlikely]] if (!check_packet(packet)) {
                    __main_logger.push("imx8m: protocol error: check_packet failed");
                    std::cout << "failed3" << std::endl;
                    continue;
                }

                /*if (!in(reinterpret_cast<sockaddr_in *>(&addr))) {
                    __main_logger.push("imx8m: protocol error: not a slave address");
                    std::cout << "failed in" << std::endl;
                    continue;
                }*/

                switch (packet.type)
                {
                case IMX8M_ACK:
                    std::cout << "ACK" << std::endl;
                    break;
                case IMX8M_REJECTED:
                    __main_logger.push("imx8m: protocol error: packet rejected");
                    std::cout << "failed4" << std::endl;
                    break;
                
                default:
                    __main_logger.push("imx8m: protocol error: packet type failed");
                    std::cout << "packet type" << std::endl;
                    break;
                }
            }
        }
    }

    void Imx8m::__do_recv(const imx8m_packet *packet, const sockaddr *addr, const socklen_t len) noexcept {
        const size_t idx = __th_pool.get_last_idx();

        [[gnu::unlikely]] if (!check_packet(packet)) {
            __th_logger_pool[idx].push("imx8m: protocol error: check_packet failed");
//#ifndef NDEBUG
            std::cout << "__do_recv::check_packet" << std::endl;
//#endif /* !NDEBUG */
        }
        /*else if (!in(addr, len)) {
            __th_logger_pool[idx].push("imx8m: protocol error: not a client ip");
//#ifndef NDEBUG
            std::cout << "__do_recv::in" << std::endl;
//#endif */ /* !NDEBUG */
        /* } */
        else {
           
            imx8m_packet p;
            bzero(&p, sizeof(p));
            p.v_major = 1;
            p.type = IMX8M_ACK;
            p.checksum = checksum(&p);

            if (!__th_sockets[idx].send(&p, sizeof(p), 0, addr, len))
                __th_logger_pool[idx].push("imx8m: system error: sendfrom");

            if (packet->type == IMX8M_PICTURE_TIMER) {
                __th_timer.queue(TimePoint(nanoseconds(packet->duration)),
                                 &Imx8m::__take_picture,
                                 this);
            }
        }
        delete packet;
        delete addr;
    }

    [[gnu::cold]]
    void Imx8m::__do_stat_connections(const size_t& nb) noexcept {
        imx8m_packet packet;    
        socklen_t len;
        sockaddr addr;

        for (size_t cidx = 0; cidx < __connections_sa.size(); ++cidx) {
            for (size_t i = 0; i < nb; ++i) {
                bzero(&packet, sizeof(packet));

                packet.v_major = 1;
                packet.type = IMX8M_TEST_CON;
                packet.checksum = checksum(&packet);

                auto start = high_resolution_clock::now();

                [[gnu::unlikely]] if (!__th_sockets[0].send(&packet,
                                                            sizeof(packet),
                                                            0,
                                                            reinterpret_cast<sockaddr *>(&__connections_sa[cidx]),
                                                            sizeof(__connections_sa[0])))
                {
                    __error_str = "imx8m: system error: sendto";
                    continue;
                }

                [[gnu::unlikely]] if (__th_sockets[0].recv(&packet,
                                                           sizeof(packet),
                                                           0,
                                                           &addr,
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