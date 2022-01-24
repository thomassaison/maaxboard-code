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
                : __socket{-1}
            {}

            UDPSocket_IpV4(const char *ip, const uint16_t port) noexcept
            {
                this->init(ip, port);
            }

            UDPSocket_IpV4(const std::string& ip, const uint16_t port) noexcept
                : UDPSocket_IpV4(ip.c_str(), port)
            {}

            void init(const char *ip, const uint16_t port) noexcept;

            void init(const std::string& ip, const uint16_t port) noexcept
            {
                this->init(ip.c_str(), port);
            }

            UDPSocket_IpV4(const UDPSocket_IpV4&) = delete;

            UDPSocket_IpV4(UDPSocket_IpV4&& other)
                : UDPSocket_IpV4()
            {
                this->swap(other);
            }

            UDPSocket_IpV4& operator=(const UDPSocket_IpV4&) = delete;
            UDPSocket_IpV4& operator=(UDPSocket_IpV4&& other) noexcept {
                this->swap(other);
                return *this;
            }

            ~UDPSocket_IpV4() noexcept {
                if (0 <= __socket)
                    close(__socket);
            }

            operator int() const noexcept {
               return __socket;
            }

            operator const char *() const noexcept {
               return __ip.c_str();
            }

            operator std::string() const noexcept {
               return __ip;
            }

            operator uint16_t() const noexcept {
               return __port;
            }

            bool operator==(const sockaddr *addr) const noexcept {
                return memcmp(&__sa, addr, sizeof(__sa));
            }

            bool operator==(const sockaddr& addr) const noexcept {
                return this->operator==(&addr);
            }

            bool operator!=(const sockaddr *addr) const noexcept {
                return !this->operator==(addr);
            }

            bool operator!=(const sockaddr& addr) const noexcept {
                return !this->operator==(addr);
            }

            bool is_ok() const noexcept {
                return __socket != -1;
            }

            void swap(UDPSocket_IpV4& other) noexcept;

            bool recv(void *buf,
                      const size_t size,
                      const int flags,
                      socklen_t *len) noexcept;

            bool send(const void *buf,
                      const size_t size,
                      const int flags) noexcept;

            bool bind() noexcept {
                return ::bind(__socket, reinterpret_cast<sockaddr *>(&__sa), sizeof(__sa)) == 0;
            }

        private:
            int         __socket = -1;
            std::string __ip;
            uint16_t    __port;
            sockaddr_in __sa;
        };
    }
}