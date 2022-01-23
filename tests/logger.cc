#include "gtest/gtest.h"
#include "imx8m/logger/logger.hh"
#include <stdio.h>

namespace{

    TEST(Logger, NewFile){
        std::string file = "test.txt";
        imx8m::logger::Logger sb(file);
        ASSERT_TRUE(sb.is_okay());
        if (unlink(file.c_str()) != 0)
            FAIL();

    }

    TEST(Logger, noFile){
        imx8m::logger::Logger log;
        EXPECT_FALSE(log.is_okay());
    }

        TEST(Logger, SimpleFile){
        imx8m::logger::Logger log;
        EXPECT_FALSE(log.is_okay());
    }
}