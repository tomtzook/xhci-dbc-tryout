cmake_minimum_required(VERSION 3.21)
project(xhci C)

set(CMAKE_C_STANDARD 11)

include_directories(include)
include_directories(xdbc)

add_library(xhci STATIC
        xdbc/xhci/ring.c
        xdbc/xdbc/xdbc.c
        xdbc/xhci/ext-cap.c
        xdbc/hw/pci.c xdbc/utils.c xdbc/xhci/dbc.c)
