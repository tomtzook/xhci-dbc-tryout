cmake_minimum_required(VERSION 3.21)
project(include/xhci)

set(CMAKE_C_STANDARD 11)

include_directories(include)

add_library(xhci
        src/xhci/ring.c
        src/xdbc/xhci-dbc.cpp
        )

add_executable(test main.c)
target_link_libraries(test xhci)
