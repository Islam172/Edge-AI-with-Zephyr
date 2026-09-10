cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test_sensor_init)

target_sources(app PRIVATE
    src/test_sensor_init.c
    ../../src/sensor/sensor_raw.c
)

target_include_directories(app PRIVATE
    ../../src
)