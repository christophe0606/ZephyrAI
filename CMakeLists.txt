# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_world)

target_sources(app PRIVATE src/main.c
  src/stream.cpp
  src/stream/scheduler/scheduler.cpp
  src/stream/cpp/cg_queue.cpp
)

target_include_directories(app PUBLIC src src/stream/scheduler 
  src/stream/cpp)