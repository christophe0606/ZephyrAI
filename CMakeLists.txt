# SPDX-License-Identifier: Apache-2.0
list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/../../../../CMSIS-Stream)
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_world)

#add_subdirectory(cmsisstream)

target_sources(app PRIVATE src/main.cpp
  src/streamgraph/scheduler/scheduler_grapha.cpp
  src/streamgraph/kws_img.c 
  src/streamgraph/cpp/kws_mfcc.c
  src/streamgraph/cpp/grapha_params.c
  src/networks/kws_micronet_m_vela_H128.tflite.cpp
  src/networks/network.cpp
)

target_include_directories(app PUBLIC src src/streamgraph/scheduler 
  src/streamgraph
  src/streamgraph/cpp
  src/networks
  )

target_compile_definitions(app PRIVATE ARM_STREAM_CUSTOM_CONFIG
   ACTIVATION_BUF_SZ=CONFIG_ACTIVATION_BUF_SZ)

