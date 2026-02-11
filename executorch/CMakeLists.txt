zephyr_library_named(et)

zephyr_library_compile_definitions(
    C10_USING_CUSTOM_GENERATED_MACROS
)

zephyr_library_compile_options(
    -ffunction-sections
    -fdata-sections
    -Wno-switch 
    -Wno-float-conversion 
    -Wno-double-promotion
    -fno-pic
    -fno-pie
)

zephyr_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/zephyr/src/arm_executor_runner.cpp
    ${CMAKE_CURRENT_LIST_DIR}/zephyr/src/arm_memory_allocator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/zephyr/src/ethos_u_stub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/et.cpp
  )



zephyr_include_directories(${CMAKE_CURRENT_LIST_DIR})

zephyr_library_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/zephyr/src
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/model
    ${CMAKE_CURRENT_LIST_DIR}/include/executorch/runtime/core/portable_type/c10
)

   add_library(executorch_core STATIC IMPORTED)
set_target_properties(executorch_core PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/libexecutorch_core.a
)

add_library(executorch STATIC IMPORTED)
set_target_properties(executorch PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/libexecutorch.a
)

# Portable kernels and ops
add_library(portable_kernels STATIC IMPORTED)
set_target_properties(portable_kernels PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/libportable_kernels.a
)

add_library(portable_ops_lib STATIC IMPORTED)
set_target_properties(portable_ops_lib PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/libportable_ops_lib.a
)

# Quantized kernels and ops
add_library(quantized_kernels STATIC IMPORTED)
set_target_properties(quantized_kernels PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/libquantized_kernels.a
)

add_library(quantized_ops_lib STATIC IMPORTED)
set_target_properties(quantized_ops_lib PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/libquantized_ops_lib.a
)

# Cortex-M optimized kernels (optional, for Cortex-M targets)
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/lib/libcortex_m_kernels.a)
  add_library(cortex_m_kernels STATIC IMPORTED)
  set_target_properties(cortex_m_kernels PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/libcortex_m_kernels.a
  )

  add_library(cortex_m_ops_lib STATIC IMPORTED)
  set_target_properties(cortex_m_ops_lib PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/libcortex_m_ops_lib.a
  )
endif()

# Ethos-U delegate (optional, for NPU acceleration)
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/lib/libexecutorch_delegate_ethos_u.a)
  add_library(executorch_delegate_ethos_u STATIC IMPORTED)
  set_target_properties(executorch_delegate_ethos_u PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/libexecutorch_delegate_ethos_u.a
  )
endif()

zephyr_library_link_libraries(
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
  zephyr_library_link_libraries(
    cortex_m_ops_lib
    cortex_m_kernels
  )
endif()

# Add Ethos-U delegate if available
# The delegate contains static initializers that register the backend with ExecuTorch
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/lib/libexecutorch_delegate_ethos_u.a)
  # Link the delegate library 
  zephyr_library_link_libraries(${CMAKE_CURRENT_LIST_DIR}/lib/libexecutorch_delegate_ethos_u.a)
  
  # Link the object files directly to ensure static constructor is included
  # The static constructor _GLOBAL__sub_I_EthosUBackend.cpp registers the backend
  if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/lib/EthosUBackend.cpp.obj)
    zephyr_library_link_libraries(
      ${CMAKE_CURRENT_LIST_DIR}/lib/EthosUBackend.cpp.obj
      ${CMAKE_CURRENT_LIST_DIR}/lib/VelaBinStream.cpp.obj
    )
  endif()
  
  # Link Zephyr's Ethos-U driver if available
  if(TARGET ethosu_core_driver)
    zephyr_library_link_libraries(ethosu_core_driver)
  endif()
endif()

