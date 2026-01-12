# SPDX-License-Identifier: Apache-2.0
# Your west file should contain something like this to include the CMSIS-Stream module:
#    - name: cmsisstream
#      url: https://github.com/ARM-software/CMSIS-Stream
#      revision: main
#      path: modules/lib/cmsisstream

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_world)

add_subdirectory(src/streamgraph/common)
add_subdirectory(src/streamgraph/streamnodes)

target_sources(app PRIVATE src/main.cpp
  src/streamgraph/nodes_csrc/kws_img.c 
  src/streamgraph/nodes_csrc/kws_mfcc.c
  src/networks/kws_micronet_m_vela_H128.tflite.cpp
  src/networks/network.cpp
)

target_include_directories(app PUBLIC 
  src/streamgraph/streamnodes
  src/streamgraph/streamnodes/appnodes
  src/networks
  )

# Appa
target_sources(app PRIVATE 
  src/streamgraph/appa/scheduler_appa.cpp
  src/streamgraph/appa/appa_params.c
  )

target_include_directories(app PUBLIC 
  src/streamgraph/appa
  )



target_link_libraries(app PRIVATE stream_nodes appcommon)