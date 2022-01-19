#pragma once

#include <vector>
#include <string>
#include <nlohmann/json.hpp>


namespace imx8m {
    namespace config {
        class ConfigHost {
        public:
            ConfigHost(const std::string& ip, const unsigned short port) noexcept
                : __ip{ip}
            {
                __port = port;
            }

            ConfigHost()  = default;
            ~ConfigHost() = default;

            void set_ip(const std::string& ip) noexcept {
                __ip = ip;
            }

            void set_port(const unsigned short port) noexcept {
                __port = port;
            }

            std::string& get_ip() noexcept {
                return __ip;
            }

            unsigned short get_port() noexcept {
                return __port;
            }

        private:
            std::string    __ip;
            unsigned short __port;
        };

        class Config {
        public:
            Config(const char *path) noexcept;

            Config(const std::string& path) noexcept
                : Config(path.c_str())
            {}

            ~Config() = default;

            bool is_ok() noexcept {
                return __error_str.empty();
            }

            bool is_master() noexcept {
                return !__is_master;
            }

            std::vector<ConfigHost>& get_connections() noexcept {
                return __connections;
            }

            ConfigHost& get_connections_idx(const size_t idx) noexcept {
                return __connections[idx];
            }

            std::string& get_self_ip() noexcept {
                return __self_host.get_ip();
            }

            unsigned short get_self_port() noexcept {
                return __self_host.get_port();
            }

            std::string& get_error_str() noexcept {
                return __error_str;
            }

        private:
            std::string             __error_str;

            bool                    __is_master = false;
            std::vector<ConfigHost> __connections;
            ConfigHost              __self_host;

            void __config_self_ip(nlohmann::json& conf) noexcept;
            void __config_self_port(nlohmann::json& conf) noexcept;
            void __config_is_master(nlohmann::json& conf) noexcept;
            void __config_master(nlohmann::json& conf) noexcept;
            void __config_default(nlohmann::json& conf) noexcept;

        };
    }
}
