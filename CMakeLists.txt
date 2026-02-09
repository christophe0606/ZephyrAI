# SPDX-License-Identifier: Apache-2.0
# Your west file should contain something like this to include the CMSIS-Stream module:
#    - name: cmsisstream
#      url: https://github.com/ARM-software/CMSIS-Stream
#      revision: main
#      path: modules/lib/cmsisstream

cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_POSITION_INDEPENDENT_CODE OFF)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_world)


add_subdirectory(src/streamgraph/common)
add_subdirectory(src/streamgraph/streamnodes)

target_sources(app PRIVATE src/main.cpp src/startup.c
  src/streamgraph/nodes_src/kws_img.c 
  src/streamgraph/nodes_src/kws_mfcc.c
  
  src/streamgraph/nodes_src/init_drv_src.cpp
  src/networks/kws_micronet_m_vela_H128.tflite.cpp
  src/container.c
  src/md5.c
  src/networks/network.cpp
)

target_compile_options(app PRIVATE
  -ffunction-sections
  -fdata-sections
  -fno-pic
  -fno-pie
)

if (CONFIG_DISPLAY)
  target_sources(app PRIVATE
    src/streamgraph/nodes_src/img_utils.cpp
  )
endif()


target_include_directories(app PUBLIC 
  src/streamgraph/streamnodes
  src/streamgraph/streamnodes/appnodes
  src/networks
  src
  )

#######################
# Appa
target_sources(app PRIVATE 
  src/streamgraph/appa/scheduler_appa.cpp
  src/streamgraph/appa/appa_params.c
  )

target_include_directories(app PUBLIC 
  src/streamgraph/appa
  )

#######################
# AppB
target_sources(app PRIVATE 
  src/streamgraph/appb/scheduler_appb.cpp
  src/streamgraph/appb/appb_params.c
  )

target_include_directories(app PUBLIC 
  src/streamgraph/appb
  )

#######################
# AppC
target_sources(app PRIVATE 
  src/streamgraph/appc/scheduler_appc.cpp
  src/streamgraph/appc/appc_params.c
  )

target_include_directories(app PUBLIC 
  src/streamgraph/appc
  )

if (CONFIG_TEMPLATE_INSTANTIATIONS)
    message(STATUS "Including template instantiations for code size optimization")
    target_sources(app PRIVATE src/streamgraph/common/template_instantiations.cpp)
endif()

target_link_libraries(app PRIVATE stream_nodes appcommon)

if (CONFIG_PREBUILT_ET)
    message(STATUS "Linking prebuilt executorch library")
    add_subdirectory(executorch)
    target_link_libraries(app PRIVATE executorch_runtime)

   add_library(executorch_core STATIC IMPORTED)
set_target_properties(executorch_core PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/executorch/lib/libexecutorch_core.a
)

add_library(executorch STATIC IMPORTED)
set_target_properties(executorch PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/executorch/lib/libexecutorch.a
)

# Portable kernels and ops
add_library(portable_kernels STATIC IMPORTED)
set_target_properties(portable_kernels PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/executorch/lib/libportable_kernels.a
)

add_library(portable_ops_lib STATIC IMPORTED)
set_target_properties(portable_ops_lib PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/executorch/lib/libportable_ops_lib.a
)

# Quantized kernels and ops
add_library(quantized_kernels STATIC IMPORTED)
set_target_properties(quantized_kernels PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/executorch/lib/libquantized_kernels.a
)

add_library(quantized_ops_lib STATIC IMPORTED)
set_target_properties(quantized_ops_lib PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/executorch/lib/libquantized_ops_lib.a
)

# Cortex-M optimized kernels (optional, for Cortex-M targets)
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/executorch/lib/libcortex_m_kernels.a)
  add_library(cortex_m_kernels STATIC IMPORTED)
  set_target_properties(cortex_m_kernels PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/executorch/lib/libcortex_m_kernels.a
  )

  add_library(cortex_m_ops_lib STATIC IMPORTED)
  set_target_properties(cortex_m_ops_lib PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/executorch/lib/libcortex_m_ops_lib.a
  )
endif()

# Ethos-U delegate (optional, for NPU acceleration)
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/executorch/lib/libexecutorch_delegate_ethos_u.a)
  add_library(executorch_delegate_ethos_u STATIC IMPORTED)
  set_target_properties(executorch_delegate_ethos_u PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/executorch/lib/libexecutorch_delegate_ethos_u.a
  )
endif()

target_link_libraries(app PRIVATE
  # Core ExecuTorch runtime
  executorch
  executorch_core
  
  # Quantized operators (used by most models)
  quantized_ops_lib
  quantized_kernels
  
  # Portable operators (fallback for non-delegated ops)
  portable_ops_lib
  portable_kernels
)

# Add Cortex-M optimized kernels if available and targeting Cortex-M
# Note: These require CMSIS-NN library, so only link if it's available
if(CONFIG_CPU_CORTEX_M AND TARGET cortex_m_ops_lib AND TARGET CMSIS_NN)
  target_link_libraries(app PRIVATE
    cortex_m_ops_lib
    cortex_m_kernels
    CMSIS_NN
  )
endif()

# Add Ethos-U delegate if available
# The delegate contains static initializers that register the backend with ExecuTorch
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/executorch/lib/libexecutorch_delegate_ethos_u.a)
  # Link the delegate library 
  target_link_libraries(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/executorch/lib/libexecutorch_delegate_ethos_u.a)
  
  # Link the object files directly to ensure static constructor is included
  # The static constructor _GLOBAL__sub_I_EthosUBackend.cpp registers the backend
  if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/executorch/lib/EthosUBackend.cpp.obj)
    target_link_libraries(app PRIVATE 
      ${CMAKE_CURRENT_LIST_DIR}/executorch/lib/EthosUBackend.cpp.obj
      ${CMAKE_CURRENT_LIST_DIR}/executorch/lib/VelaBinStream.cpp.obj
    )
  endif()
  
  # Link Zephyr's Ethos-U driver if available
  if(TARGET ethosu_core_driver)
    target_link_libraries(app PRIVATE ethosu_core_driver)
  endif()
endif()


endif()