#include <iostream>
#include <fstream>

#include "config.hh"

namespace imx8m {
    namespace config {

        void Config::__config_is_master(nlohmann::json& conf) noexcept {
            [[gnu::unlikely]] if (!conf.is_boolean()) {
                __error_str = "\'is-master\' should be a boolean value";
                return;
            }

            __is_master = conf;
        }

        void Config::__config_self_port(nlohmann::json& conf) noexcept {
            [[gnu::unlikely]] if (!conf.is_number_unsigned()) {
                __error_str = "\'port\' should be an unsigned number";
                return;
            }

            __self_host.set_port(conf);
        }

        void Config::__config_self_ip(nlohmann::json& conf) noexcept {
            [[gnu::unlikely]] if (!conf.is_string()) {
                __error_str = "\'ip\' should be a string";
                return;
            }

            __self_host.set_ip(conf);
        }

        void Config::__config_master(nlohmann::json& conf) noexcept {
            bool found_port, found_ip;
            unsigned short port;
            std::string ip;

            for(auto it0 = conf.begin(); it0 != conf.end(); ++it0)
            {
                /* it.key cannot throw at this time */
                if (it0.key() == "slaves") {

                    [[gnu::unlikely]] if (!it0.value().is_array()) {
                        __error_str = "\'slaves\' should be an array";
                        return;
                    }

                    for (auto it1 = it0.value().begin(); it1 != it0.value().end(); ++it1)
                    {
                        [[gnu::unlikely]] if (!it1.value().is_object()) {
                            __error_str = "\'clients\' value should be an object";
                            return;
                        }

                        found_port = false;
                        found_ip   = false;

                        for (auto it2 = it1.value().begin();
                             it2 != it1.value().end() && (!found_port || !found_ip);
                             ++it2)
                        {
                            if (it2.key() == "ip") {
                                [[gnu::unlikely]] if (!it2.value().is_string()) {
                                    __error_str = "\'ip\' should be a string";
                                    return;
                                }
                                ip = it2.value();
                                found_ip = true;
                            }
                            else if (it2.key() == "port") {
                                [[gnu::unlikely]] if (!it2.value().is_number_unsigned()) {
                                    __error_str = "\'port\' should be an unsigned number";
                                    return;
                                }
                                port = it2.value();
                                found_port = true;
                            }
                        }

                        if (!found_ip) {
                            __error_str = "A client ip is undefine";
                            return;
                        }
                        if (!found_port) {
                            __error_str = "A client port is undefine";
                            return;
                        }

                        __connections.push_back(ConfigHost(ip, port));
                    }

                    break;
                }
            }

            if (__connections.empty()) {
                __error_str = "Master don't have any clients";
                return;
            }
        }

        void Config::__config_default(nlohmann::json& conf) noexcept {
            bool found_port, found_ip;
            std::string ip;

            unsigned short port = 0;

	    found_port = false;
	    found_ip = false;

            for(auto it0 = conf.begin(); it0 != conf.end(); ++it0) {
                
                if (it0.key() == "master") {

                    [[gnu::unlikely]] if (!it0.value().is_object()) {
                        __error_str = "\'master\' should be a JSON object";
                        return;
                    }

                    for (auto it1 = it0.value().begin();
                         it1 != it0.value().end() && (!found_ip || !found_port);
                         ++it1)
                    {
                        if (it1.key() == "ip") {
                            [[gnu::unlikely]] if (!it1.value().is_string()) {
                                __error_str = "\'ip\' should be a string";
                                return;
                            }

                            ip = it1.value();
                            found_ip = true;
                        }
                        else if (it1.key() == "port") {
                            [[gnu::unlikely]] if (!it1.value().is_number_unsigned()) {
                                __error_str = "\'port\' should be an unsigned number";
                                return;
                            }

                            port = it1.value();
                            found_port = true;
                        }
                    }
                    
                    if (!found_ip) {
                        __error_str = "Master ip is undefine";
                        return;
                    }
                    
                    if (!found_port) {
                        __error_str = "Master port is undefine";
                        return;
                    }

                    /* port is init at this time */

                    __connections.push_back(ConfigHost(ip, port));
                    break;
                }
            }

            if (__connections.empty()) {
                __error_str = "\'master\' value not found";
                return;
            }
        }

        Config::Config(const char *path) noexcept {
            bool           found_master,
                           found_ip,
                           found_port;

            nlohmann::json json_conf;

            {
                std::ifstream stream(path);
		if (!stream.good()) {
			__error_str = "Cannot open config";
			return;
		}
                stream >> json_conf;
            }

            /* Config begin by an object */
            [[gnu::unlikely]] if (!json_conf.is_object())
            {
                __error_str = "Config should begin by a JSON object";
                return;
            }

            found_master   = false;
            found_ip       = false;
            found_port     = false;


            for(auto it0 = json_conf.begin();
                it0 != json_conf.end()
                && (!found_master || !found_port || !found_ip);
                ++it0)
            {
                /* it.key cannot throw at this time */
                if (it0.key() == "is-master") {
                    __config_is_master(it0.value());
                    found_master = true;
		    std::cout << __is_master << std::endl;
                }
                else if (it0.key() == "ip") {
                    __config_self_ip(it0.value());
                    found_ip = true;
                }
                else if (it0.key() == "port") {
                    __config_self_port(it0.value());
                    found_port = true;
                }

		std::cout << it0.key() << std::endl;
            }

            [[gnu::unlikely]] if (!found_ip) {
                __error_str = "\'ip\' definition not found";
                return;
            }

            [[gnu::unlikely]] if (!found_port) {
                __error_str = "\'port\' definition not found";
                return;
            }

            if (__is_master)
                __config_master(json_conf);
            else
                __config_default(json_conf);
        }
    }
}
