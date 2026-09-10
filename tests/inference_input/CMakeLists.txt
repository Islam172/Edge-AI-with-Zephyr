cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test_inference_input)

target_sources(app PRIVATE
    src/test_inference_input.c
    ../../src/inference_engine/emlearn/emlearn_model.c
)

target_include_directories(app PRIVATE
    ../../src
)