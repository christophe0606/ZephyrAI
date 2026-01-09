cmake_minimum_required(VERSION 3.20)

add_library(appa STATIC grapha_params.c 
  scheduler_grapha.cpp)

target_link_libraries(appa PRIVATE zephyr_interface 
  stream_nodes 
  appcommon)
target_include_directories(appa PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})