#include "gtest/gtest.h"
#include "imx8m/logger/logger.hh"
#include <stdio.h>

namespace{

    const std::string testfile = "res/test.log";

    void checkContent(std::string &expectedOutput){
        std::ifstream t(testfile);
        std::string genfile((std::istreambuf_iterator<char>(t)),
                             std::istreambuf_iterator<char>());
        ASSERT_EQ(expectedOutput,genfile) << "EXPECTED :" << expectedOutput << " GOT:" << genfile << std::endl;
    }

    void destroyfile(imx8m::logger::Logger &logger){
        EXPECT_TRUE(logger.is_ok());
        if (unlink(testfile.c_str()) != 0)
            FAIL();
        else
            SUCCEED();
    }

    TEST(Logger, NewFile){
        std::string file = "test.txt";
        imx8m::logger::Logger sb(file);
        ASSERT_TRUE(sb.is_ok());
        if (unlink(file.c_str()) != 0)
            FAIL();

    }

    TEST(Logger, noFile){
        imx8m::logger::Logger log;
        EXPECT_FALSE(log.is_ok());
    }

    TEST(Logger, SimpleFile){
        imx8m::logger::Logger log;
        EXPECT_FALSE(log.is_ok());
    }

    TEST(Logger, fileExist){
        imx8m::logger::Logger log(testfile);
        EXPECT_TRUE(log.is_ok());
    }

    TEST(Logger, simplepush){
        imx8m::logger::Logger log(testfile);
        EXPECT_TRUE(log.is_ok());
        
        log.push("this is a test");
        destroyfile(log);
     }

    TEST(Logger, simpleWrite){
        imx8m::logger::Logger log(testfile);
        EXPECT_TRUE(log.is_ok());
        std::string content = "this is a test";
        log.push(content);
        log.debug();
        ASSERT_TRUE(log.is_ok());

        log.flush();
        ASSERT_TRUE(log.is_ok());

       checkContent(content+='\n');
       //destroyfile(log);
     }
}