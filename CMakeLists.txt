cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(fan_anomaly_detection)

target_sources(app PRIVATE
    src/main.c
    src/sensor/sensor_raw.c
    src/sensor/sensor_collect.c
)

if(CONFIG_SENSOR_COLLECT_RUN_INFERENCE)
  if(CONFIG_SENSOR_INFENG_EMLEARN)
    target_sources(app PRIVATE
        src/inference_engine/emlearn/emlearn_model.c
    )
    target_include_directories(app PRIVATE
        ${ZEPHYR_EMLEARN_MODULE_DIR}
    )
  endif()
endif()

target_include_directories(app PRIVATE src)