cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})
project(central_hr)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

zephyr_library_include_directories(${ZEPHYR_BASE}/samples/bluetooth)
