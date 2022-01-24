#include <iostream>

#include <string.h>

#include "udp_socket_ipv4.hh"

namespace imx8m {
    namespace network {

        void UDPSocket_IpV4::swap(UDPSocket_IpV4& other) noexcept {
            std::swap(__socket, other.__socket);
        }

        bool UDPSocket_IpV4::recv(void *buf,
                                  const size_t size,
                                  const int flags,
                                  sockaddr *addr,
                                  socklen_t *len) noexcept
        {
            [[gnu::unlikely]] while ((recvfrom(__socket,
                                               buf,
                                               size,
                                               flags,
                                               addr,
                                               len) == -1))
            {
                if (/*errno != EAGAIN && errno != EWOULDBLOCK &&*/ errno != EINTR)
                    return false;
                //std::cout << "recv " << errno  << std::endl;
            }
            return true;
        }

        bool UDPSocket_IpV4::send(const void *buf,
                                  const size_t size,
                                  const int flags,
                                  const sockaddr *addr,
                                  const socklen_t len) noexcept
        {
            [[gnu::unlikely]] while ((sendto(__socket,
                                             buf,
                                             size,
                                             flags,
                                             addr,
                                             len) == -1))
            {
                if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
                    return false;
                std::cout << "send " << errno  << std::endl;
            }
            return true;
        }

    }
}