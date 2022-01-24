#include <gtest/gtest.h>

#include "imx8m/config/config.hh"

using namespace imx8m::config;
namespace {

    TEST(Config, isMaster) {
        auto configMaster = Config("example/config-master.json");
        auto configDefault = Config("example/config-default.json");

	    if (!configDefault.is_ok()) {
		    std::cerr << configDefault.get_error_str() << std::endl;
		    FAIL();
	    }

	    if (!configMaster.is_ok()) {
		    std::cerr << configMaster.get_error_str() << std::endl;
		    FAIL();
	    }
        
        EXPECT_TRUE(configMaster.is_master());
        EXPECT_FALSE(configDefault.is_master());

        EXPECT_EQ(configMaster.get_connections().size(), 1);
        EXPECT_EQ(configDefault.get_connections().size(), 1);
    }

    TEST(Config, wrongJson) {
        auto config = Config("failure.json"); //This doesn't exist
        
        if (!config.is_ok())
            SUCCEED();
        else
            FAIL();
    }

    TEST(Config, checkPortsValue) {
        auto configMaster = Config("example/config-master.json");
        auto configDefault = Config("example/config-default.json");
        
	    if (!configDefault.is_ok()) {
	    	std::cerr << configDefault.get_error_str() << std::endl;
	    	FAIL();
	    }
    
	    if (!configMaster.is_ok()) {
	    	std::cerr << configMaster.get_error_str() << std::endl;
	    	FAIL();
	    }
        
        EXPECT_EQ(configMaster.get_self_port(), 8080);
        EXPECT_EQ(configDefault.get_self_port(), 8081);
    }

    TEST(Config, checkIpsValue) {
        auto configMaster = Config("example/config-master.json");
        auto configDefault = Config("example/config-default.json");

        if (!configDefault.is_ok()) {
		    std::cerr << configDefault.get_error_str() << std::endl;
		    FAIL();
	    }

	    if (!configMaster.is_ok()) {
	    	std::cerr << configMaster.get_error_str() << std::endl;
	    	FAIL();
	    }
        
        EXPECT_EQ(configMaster.get_self_ip(), "127.0.0.1");
        EXPECT_EQ(configDefault.get_self_ip(), "127.0.0.1");

    }

    TEST(Config, checkClient) {
        auto configMaster = Config("example/config-master.json");
        auto configDefault = Config("example/config-default.json");

	    if (!configDefault.is_ok()) {
	    	std::cerr << configDefault.get_error_str() << std::endl;
	    	FAIL();
	    }

	    if (!configMaster.is_ok()) {
	    	std::cerr << configMaster.get_error_str() << std::endl;
	    	FAIL();
	    }

        auto masterClient = configMaster.get_connections_idx(0);
        auto defaultClient = configDefault.get_connections_idx(0);
        

        EXPECT_EQ(masterClient.get_port(), 8081);
        EXPECT_EQ(defaultClient.get_port(), 8080);

        EXPECT_EQ(masterClient.get_ip(), "127.0.0.1");
        EXPECT_EQ(defaultClient.get_ip(), "127.0.0.1");
    }
}
