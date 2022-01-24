cmake_minimum_required(VERSION 3.16)

option(DEBUG_MODE "Compile in debug mode" OFF)

option(ENABLE_TESTING "enabling tests" ON)

# Don't try to enable this option, it doesn't work now
option(ARM_CROSS_COMPILE "Cross compile for linux arm system" OFF)

# This option let you to choose between clang++ and g++ to compile this project.
# Clang can give some realy usefull warnings where g++ doesn't.
# So try to compile with clang sometime and you will produce better code.

# /!\ Don't forget, g++ have better code optimization than clang, so to release mode use g++ is better
option(USE_CLANG "Compile project with clang compiler" OFF)

option(TESTING "Enable google test" OFF)

# We compile for linux, don't care other thing
set(CMAKE_SYSTEM_NAME Linux)

if (ARM_CROSS_COMPILE)
    set(CMAKE_SYSTEM_PROCESSOR arm)
    message("-- ARM cross compile enabled")
endif(ARM_CROSS_COMPILE)

if (USE_CLANG)
	set(CMAKE_CXX_COMPILER /usr/bin/clang++)

    # This enable a lot of compiler warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything")

    # Suppress unused warning
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat")          # Don't care about c++98
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat-pedantic") # Don't care about c++98

    # This is a "system" code, so you need to manage padding by your self (attribute packed is your friend if you don't need pading)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-padded")

    # Clag special warning (maybe in -Weverything but force it because really usefull)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wweak-vtables")

    # If have weak vtable you MUST remove this warning
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=weak-vtables")

	if (ARM_CROSS_COMPILE) # Don't care, it doesn't work
		set(CMAKE_CXX_COMPILER_TARGET arm-linux-gnueabi)
	endif(ARM_CROSS_COMPILE)

else()
	set(CMAKE_CXX_COMPILER g++)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-usage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat-overflow")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat-truncation")
endif(USE_CLANG)

set(CMAKE_CXX_COMPILER_WORKS TRUE)

# CXXFLAGS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdouble-promotion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat=2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat-security")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wundef")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-common")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wold-style-cast")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsign-conversion")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wmissing-noreturn")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnonnull")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused")

# Some warnings whose could'nt be ignored
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=nonnull")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=unused")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=old-style-cast")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=missing-noreturn")

if (DEBUG_MODE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
else()
    add_definitions(-DNDEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif(DEBUG_MODE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(imx8m
        VERSION 1.0
        DESCRIPTION "An embedded program for linux to synchronize the picture taking between several imx8m, in real time"
        LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
    GIT_TAG v3.7.3)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
    FetchContent_Populate(json)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

set(IMX8M_SRC
    src/main.cc
    src/imx8m/imx8m.cc
    src/imx8m/config/config.cc
    src/imx8m/thread/thread.cc
    src/imx8m/thread/thread_pool.cc
    src/imx8m/thread/timer_thread.cc
    src/imx8m/network/udp_socket_ipv4.cc
    src/imx8m/logger/logger.cc)

set(IMX8M_HDR
    src/imx8m/imx8m.hh
    src/imx8m/config/config.hh
    src/imx8m/thread/thread.hh
    src/imx8m/thread/thread_pool.hh
    src/imx8m/thread/timer_thread.hh
    src/imx8m/network/udp_socket_ipv4.hh
    src/imx8m/logger/logger.hh)

add_executable(imx8m ${IMX8M_SRC} ${IMX8M_HDR})

target_link_libraries(
    imx8m 
    PRIVATE nlohmann_json::nlohmann_json
            Threads::Threads
)

target_include_directories(imx8m PUBLIC src)

message("-- C++ compiler flags: ${CMAKE_CXX_FLAGS}")

install(TARGETS imx8m DESTINATION bin)
if(ENABLE_TESTING)

    # if(GTest_FOUND)
    #     find_package(GTest REQUIRED)
    #     include_directories(${GTEST_INCLUDE_DIRS})
    # else()
    #     FetchContent_Declare(
    #         googletest
    #         URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
    #     )

    #     FetchContent_MakeAvailable(googletest)
    #     include(GoogleTest)
    # endif()

	enable_testing()
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})

	include(GoogleTest)

	add_executable(
        tconfig
        tests/config.cc
        src/imx8m/config/config.hh
        src/imx8m/config/config.cc
    )

    add_executable(
        tlogger
        tests/logger.cc
        src/imx8m/logger/logger.hh
        src/imx8m/logger/logger.cc
    )

    add_executable(
        tthread
        tests/thread.cc
        src/imx8m/thread/thread.hh
        src/imx8m/thread/thread.cc
    )

    add_executable(
        tthread_pool
        tests/thread_pool.cc
        src/imx8m/thread/thread.hh
        src/imx8m/thread/thread.cc
        src/imx8m/thread/thread_pool.hh
        src/imx8m/thread/thread_pool.cc
    )

    add_executable(
        ttimer_thread
        tests/timer_thread.cc
        src/imx8m/thread/timer_thread.hh
        src/imx8m/thread/timer_thread.cc
    )

	gtest_add_tests(
        TARGET            tconfig
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		TEST_LIST         TEST_CONFIG
    )

    gtest_add_tests(
        TARGET            tlogger
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		TEST_LIST         TEST_LOGGER
    )

    gtest_add_tests(
        TARGET            tthread
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		TEST_LIST         TEST_THREAD
    )

    gtest_add_tests(
        TARGET            tthread_pool
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		TEST_LIST         TEST_THREAD_POOL
    )

    gtest_add_tests(
        TARGET            ttimer_thread
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		TEST_LIST         TEST_TIMER_THREAD
    )

	set_tests_properties(${TEST_CONFIG} PROPERTIES TIMEOUT 30)
    set_tests_properties(${TEST_LOGGER} PROPERTIES TIMEOUT 30)
    set_tests_properties(${TEST_THREAD} PROPERTIES TIMEOUT 30)
    set_tests_properties(${TEST_THREAD_POOL} PROPERTIES TIMEOUT 30)
    set_tests_properties(${TEST_TIMER_THREAD} PROPERTIES TIMEOUT 30)

	target_link_libraries(tconfig PRIVATE nlohmann_json::nlohmann_json ${GTEST_BOTH_LIBRARIES})
    target_link_libraries(tlogger PRIVATE ${GTEST_BOTH_LIBRARIES})
    target_link_libraries(tthread PRIVATE ${GTEST_BOTH_LIBRARIES} Threads::Threads )
    target_link_libraries(tthread_pool PRIVATE ${GTEST_BOTH_LIBRARIES} Threads::Threads)
    target_link_libraries(ttimer_thread PRIVATE ${GTEST_BOTH_LIBRARIES} Threads::Threads)

    target_include_directories(tconfig PUBLIC src)
    target_include_directories(tlogger PUBLIC src)
	target_include_directories(tthread PUBLIC src)
	target_include_directories(tthread_pool PUBLIC src)
    target_include_directories(ttimer_thread PUBLIC src)

endif()
