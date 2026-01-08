if(CONFIG_CMSIS_DSP)
zephyr_library()

zephyr_library_sources(
    zephyr_runtime/event_queue.cpp
    zephyr_runtime/runtime_init.cpp
)

zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/zephyr_runtime
  ${CMAKE_CURRENT_SOURCE_DIR})
endif()