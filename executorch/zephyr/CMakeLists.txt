# Copyright (c) 2025 Arm Limited and/or its affiliates.
# SPDX-License-Identifier: Apache-2.0

# This file is included by Zephyr's module system when executorch is registered
# as a Zephyr module. It provides the ExecuTorch libraries and includes for
# Zephyr applications.

if(CONFIG_EXECUTORCH)
  # Set the path to the ai_layer engine
  set(EXECUTORCH_AI_LAYER_DIR ${CMAKE_CURRENT_LIST_DIR}/../executorch)
  
  # Add include paths from the ai_layer engine
  zephyr_include_directories(
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include/executorch
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include/executorch/runtime
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include/executorch/runtime/core
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include/executorch/runtime/executor
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include/executorch/extension
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include/executorch/extension/data_loader
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include/executorch/extension/memory_allocator
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include/executorch/extension/runner_util
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include/executorch/util
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include/executorch/schema
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include/executorch/devtools
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include/executorch/devtools/etdump
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include/executorch/runtime/core/portable_type/c10
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include/executorch/kernels
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include/executorch/kernels/portable
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include/executorch/backends
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include/executorch/backends/xnnpack
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include/executorch/backends/cortex_m/ops
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include/third-party
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include/third-party/flatbuffers
    ${EXECUTORCH_AI_LAYER_DIR}/engine/include/third-party/flatbuffers/include
    ${EXECUTORCH_AI_LAYER_DIR}/model
  )

  # Link the pre-built ExecuTorch libraries
  # Note: These libraries are built by the Docker workflow and placed in ai_layer/engine/lib
  
  set(EXECUTORCH_LIB_DIR ${EXECUTORCH_AI_LAYER_DIR}/engine/lib)
  
  if(EXISTS ${EXECUTORCH_LIB_DIR}/libexecutorch_core.a)
    # Import pre-built libraries
    add_library(executorch_core STATIC IMPORTED GLOBAL)
    set_target_properties(executorch_core PROPERTIES
      IMPORTED_LOCATION ${EXECUTORCH_LIB_DIR}/libexecutorch_core.a
    )
    
    add_library(executorch STATIC IMPORTED GLOBAL)
    set_target_properties(executorch PROPERTIES
      IMPORTED_LOCATION ${EXECUTORCH_LIB_DIR}/libexecutorch.a
    )
    
    add_library(portable_ops_lib STATIC IMPORTED GLOBAL)
    set_target_properties(portable_ops_lib PROPERTIES
      IMPORTED_LOCATION ${EXECUTORCH_LIB_DIR}/libportable_ops_lib.a
    )
    
    add_library(portable_kernels STATIC IMPORTED GLOBAL)
    set_target_properties(portable_kernels PROPERTIES
      IMPORTED_LOCATION ${EXECUTORCH_LIB_DIR}/libportable_kernels.a
    )
    
    if(EXISTS ${EXECUTORCH_LIB_DIR}/libcortex_m_ops_lib.a)
      add_library(cortex_m_ops_lib STATIC IMPORTED GLOBAL)
      set_target_properties(cortex_m_ops_lib PROPERTIES
        IMPORTED_LOCATION ${EXECUTORCH_LIB_DIR}/libcortex_m_ops_lib.a
      )
      
      add_library(cortex_m_kernels STATIC IMPORTED GLOBAL)
      set_target_properties(cortex_m_kernels PROPERTIES
        IMPORTED_LOCATION ${EXECUTORCH_LIB_DIR}/libcortex_m_kernels.a
      )
    endif()
    
    if(EXISTS ${EXECUTORCH_LIB_DIR}/libquantized_ops_lib.a)
      add_library(quantized_ops_lib STATIC IMPORTED GLOBAL)
      set_target_properties(quantized_ops_lib PROPERTIES
        IMPORTED_LOCATION ${EXECUTORCH_LIB_DIR}/libquantized_ops_lib.a
      )
      
      add_library(quantized_kernels STATIC IMPORTED GLOBAL)
      set_target_properties(quantized_kernels PROPERTIES
        IMPORTED_LOCATION ${EXECUTORCH_LIB_DIR}/libquantized_kernels.a
      )
    endif()
    
    if(EXISTS ${EXECUTORCH_LIB_DIR}/libexecutorch_delegate_ethos_u.a)
      add_library(executorch_delegate_ethos_u STATIC IMPORTED GLOBAL)
      set_target_properties(executorch_delegate_ethos_u PROPERTIES
        IMPORTED_LOCATION ${EXECUTORCH_LIB_DIR}/libexecutorch_delegate_ethos_u.a
      )
    endif()
    
    message(STATUS "ExecuTorch: Using pre-built libraries from ${EXECUTORCH_LIB_DIR}")
  else()
    message(WARNING "ExecuTorch: Pre-built libraries not found in ${EXECUTORCH_LIB_DIR}. "
                    "Run the Docker AI Layer Generation task first.")
  endif()
endif()
