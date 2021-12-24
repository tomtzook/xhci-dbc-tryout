cmake_minimum_required(VERSION 3.21)
project(xhci C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

add_library(xhci
        src/xhci/ring.c
        src/xdbc/xdbc.c
        src/xdbc/xdbc-ring.c src/xhci/ext-cap.c)

add_executable(test main.c)
target_link_libraries(test xhci)
