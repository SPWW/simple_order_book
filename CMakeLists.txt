cmake_minimum_required(VERSION 3.16)
project(lh)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

add_executable(lh
        main.cpp
        orderbook.h test.cpp)
