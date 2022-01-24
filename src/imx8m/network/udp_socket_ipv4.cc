#include "udp_socket_ipv4.hh"

#include <string.h>


namespace imx8m {
    namespace network {
        void UDPSocket_IpV4::init(const char *ip, const uint16_t port) noexcept
        {
            __ip = ip;
            __port = port;

            bzero(&__sa, sizeof(__sa));

            __sa.sin_family = AF_INET;
            __sa.sin_port = htons(__port);
            if (inet_pton(AF_INET, ip, &__sa.sin_addr) != 1)
                return;

            __socket = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
        }

        void UDPSocket_IpV4::swap(UDPSocket_IpV4& other) noexcept {
            std::swap(__socket, other.__socket);
            std::swap(__ip, other.__ip);
            std::swap(__port, other.__port);
            std::swap(__sa, other.__sa);
        }

        bool UDPSocket_IpV4::recv(void *buf,
                                  const size_t size,
                                  const int flags,
                                  socklen_t *len) noexcept
        {
            [[gnu::unlikely]] while ((recvfrom(__socket,
                                               buf,
                                               size,
                                               flags,
                                               reinterpret_cast<sockaddr *>(&__sa),
                                               len) == -1))
            {
                if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
                    return false;
            }
            return true;
        }

        bool UDPSocket_IpV4::send(const void *buf,
                                  const size_t size,
                                  const int flags) noexcept
        {
            [[gnu::unlikely]] while ((sendto(__socket,
                                             buf,
                                             size,
                                             flags,
                                             reinterpret_cast<sockaddr *>(&__sa),
                                             sizeof(__sa)) == -1))
            {
                if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
                    return false;
            }
            return true;
        }


    }
}