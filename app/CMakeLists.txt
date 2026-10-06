cmake_minimum_required(VERSION 3.20.0)

list(APPEND BOARD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/modules/our_drivers")
find_package(Zephyr REQUIRED)
project(my_zephyr_app)

target_sources(app PRIVATE src/main.cpp)
