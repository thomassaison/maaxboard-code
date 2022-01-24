#pragma once

#include <string>
#include <cstring>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

namespace imx8m {
    namespace network {
        class UDPSocket_IpV4
        {
        public:
            UDPSocket_IpV4() noexcept
                : __socket{socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)}
            {}

            UDPSocket_IpV4(const UDPSocket_IpV4&) = delete;

            UDPSocket_IpV4(UDPSocket_IpV4&& other)
                : UDPSocket_IpV4()
            {
                this->swap(other);
            }

            ~UDPSocket_IpV4() noexcept {
                if (0 <= __socket)
                    close(__socket);
            }

            operator int() const noexcept {
               return __socket;
            }

            bool is_ok() const noexcept {
                return __socket != -1;
            }

            void swap(UDPSocket_IpV4& other) noexcept;

            bool recv(void *buf,
                      const size_t size,
                      const int flags,
                      sockaddr *addr,
                      socklen_t *len) noexcept;

            bool send(const void *buf,
                      const size_t size,
                      const int flags,
                      const sockaddr *addr,
                      const socklen_t len) noexcept;

            bool bind(const sockaddr *sa, const socklen_t len) noexcept {
                return ::bind(__socket, sa, len) == 0;
            }
            
        private:
            int __socket;
        };
    }
}