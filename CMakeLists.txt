# SPDX-License-Identifier: Apache-2.0
list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/../../../../CMSIS-Stream)
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_world)

add_subdirectory(src/streamgraph/appa)
add_subdirectory(src/streamgraph/common)
add_subdirectory(src/streamgraph/streamnodes)

target_sources(app PRIVATE src/main.cpp
  src/streamgraph/c/kws_img.c 
  src/streamgraph/c/kws_mfcc.c
  src/networks/kws_micronet_m_vela_H128.tflite.cpp
  src/networks/network.cpp
)

target_include_directories(app PUBLIC 
  src/streamgraph/streamnodes
  src/streamgraph/streamnodes/appnodes
  src/streamgraph/c
  src/networks
  )

target_link_libraries(app PRIVATE appa)
target_link_libraries(app PRIVATE stream_nodes appcommon)