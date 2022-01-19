#include <iostream>

#include <string.h>
#include <unistd.h>

#include "imx8m.hh"

#include "config/config.hh"

namespace imx8m {

    thread::TimerThread timer_th;

    Imx8m::Imx8m(const char *path) noexcept {
        struct sched_param param;
        struct epoll_event ev;

        config::Config conf(path);

        if (!conf.is_ok()) {
            std::cerr << "imx8m: "
                      << path
                      << ": "
                      << conf.get_error_str()
                      << std::endl;

            exit(EXIT_FAILURE);
        }

        /* INIT SELF SOCKET */

        bzero(&__sa, sizeof(__sa));

        __sa.sin_family = AF_INET;
        [[gnu::unlikely]] if (inet_pton(AF_INET,
                                        conf.get_self_ip().c_str(),
                                        &__sa.sin_addr) != 1)
        {
            std::cerr << "imx8m: "
                      << path
                      << ": "
                      << conf.get_self_ip()
                      << ": is not a valid ipv4 address"
                      << std::endl;

            exit(EXIT_FAILURE);
        }
        __sa.sin_port = htons(conf.get_self_port());

        [[gnu::unlikely]] if ((__my_socket = socket(AF_INET,
                                                    SOCK_DGRAM | SOCK_NONBLOCK,
                                                    0)) < 0)
        {
            std::cerr << "imx8m: system error: socket" << std::endl;
            exit(EXIT_FAILURE);
        }

        [[gnu::unlikely]] if (bind(__my_socket,
                              reinterpret_cast<struct sockaddr *>(&__sa),
                              sizeof(__sa)) < 0)
        {
            std::cerr << "imx8m: system error: bind" << std::endl;
            exit(EXIT_FAILURE);
        }

        /* INIT EPOLL */

        [[gnu::unlikely]] if ((__epoll_fd = epoll_create1(0)) == -1) {
            std::cerr << "imx8m: system error: epoll_create1" << std::endl;
            exit(EXIT_FAILURE);
        }

        ev.data.fd = __my_socket;
        ev.events = EPOLLIN;
        if (epoll_ctl(__epoll_fd, EPOLL_CTL_ADD, __my_socket, &ev) == -1) {
            std::cerr << "imx8m: system error: epoll_ctl" << std::endl;
            exit(EXIT_FAILURE);
        }

        /* INIT THREAD */

        param.sched_priority = IMX8M_DEFAULT_THPRIO;
        [[gnu::unlikely]] if (pthread_setschedparam(pthread_self(),
                                                    SCHED_FIFO,
                                                    &param) != 0
                              || !__th_pool.set_sched(SCHED_FIFO,
                                                      IMX8M_DEFAULT_THPRIO + 1)
                              || !timer_th.set_sched(SCHED_FIFO,
                                                     IMX8M_DEFAULT_THPRIO + 5))
        {
            std::cerr << "imx8m: system error: pthread_setschedparam"
                      << std::endl;

            exit(EXIT_FAILURE);
        }

        /* TODO: __th_picture = std::thread(); epoll_event; etc... */
    }
    
    void Imx8m::__run_default() noexcept {
        int nfd;
        ssize_t ret;
        socklen_t socklen;

        for (;;) {

            while ((nfd = epoll_wait(__epoll_fd,
                                  __events,
                                  sizeof(__events) / sizeof(*__events),
                                  -1)) == -1)
            {
                if (errno == EINTR)
                    continue;
                std::cerr << "imx8m: system error: epoll_wait" << std::endl;
                exit(EXIT_FAILURE);
            }

            for (int i = 0; i < nfd; ++i) {

                [[gnu::unlikely]] while ((ret = recvfrom(__my_socket,
                                                         NULL /* TODO: buffer*/,
                                                         0 /* TODO: len */,
                                                         0 /* flags */,
                                                         reinterpret_cast<struct sockaddr *>(&__sa),
                                                         &socklen) == -1))
                {
                    if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
                        break;
                }

                [[gnu::unlikely]] if (ret == -1)
                    break;

                __th_pool.queue(&Imx8m::__do_nothing, this);

                /* TODO: let thread pool to process value */
                
            }
        }
        // TODO
    }

    void Imx8m::__run_master() noexcept {
        // TODO
    }
}