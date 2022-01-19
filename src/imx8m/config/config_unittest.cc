#include "gtest/gtest.h"
#include "config.hh"

//architecture example from : https://github.com/google/googletest/blob/main/googletest/samples/sample4_unittest.cc
namespace {

    TEST(Config, isMaster){
        auto configMaster = Config("config-master.json");
        auto configDefault = Config("config-default.json");
        
        EXPECT_TRUE(configMaster.is_master());
        EXPECT_FALSE(configDefault.is_master());

        EXPECT_TRUE(masterClient.is_okay());
        EXPECT_TRUE(defaultClient.is_okay());
    }

    TEST(Config, wrongJson){
        auto config = Config("failure.json"); //This doesn't exist
        
        if (!config.is_okay())
            SUCCEED();
        else
            FAIL();
    }

    TEST(Config, checkPortsValue){
        auto configMaster = Config("config-master.json");
        auto configDefault = Config("config-default.json");
        
        EXPECT_EQ(configMaster.get_port(), 8080);
        EXPECT_EQ(configDefault.get_port(), 8081);

        EXPECT_TRUE(masterClient.is_okay());
        EXPECT_TRUE(defaultClient.is_okay());
    }

        TEST(Config, checkIpsValue){
        auto configMaster = Config("config-master.json");
        auto configDefault = Config("config-default.json");
        
        EXPECT_EQ(configMaster.get_ip(), "127.0.0.1");
        EXPECT_EQ(configDefault.get_ip(), "127.0.0.1");

        EXPECT_TRUE(masterClient.is_okay());
        EXPECT_TRUE(defaultClient.is_okay());
    }

    TEST(Config, checkClient){
        auto configMaster = Config("config-master.json");
        auto configDefault = Config("config-default.json");

        auto masterClient = configMaster.get_connections_idx(0);
        auto defaultClient = configDefault.get_connections_idx(0);

        EXPECT_EQ(masterClient.get_port(), 8081);
        EXPECT_EQ(defaultClient.get_port(), 8080);

        EXPECT_EQ(masterClient.get_ip(), "127.0.0.1");
        EXPECT_EQ(defaultClient.get_ip(), "127.0.0.1");

        EXPECT_TRUE(masterClient.is_okay());
        EXPECT_TRUE(defaultClient.is_okay());
    }
}
