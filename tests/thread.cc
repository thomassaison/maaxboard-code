#include <gtest/gtest.h>

#include "imx8m/config/config.hh"

using namespace imx8m::thread;

//architecture example from : https://github.com/google/googletest/blob/main/googletest/samples/sample4_unittest.cc
namespace {
    TEST(Thread, constructors) {
        Thread t1;
        EXPECT_FALSE(t1.in_work());

        Thread t2(std::move(t1));
        EXPECT_FALSE(t2.in_work());
    }
}