


function(link_et_libs APP ROOT)


target_include_directories(${APP} PRIVATE
    ${ROOT}
    ${ROOT}/zephyr/src
    ${ROOT}/include
    ${ROOT}/model
    ${ROOT}/include/executorch/runtime/core/portable_type/c10
)


target_compile_definitions(${APP} PRIVATE
    C10_USING_CUSTOM_GENERATED_MACROS
    C10_MOBILE=1
)

target_compile_options(${APP} PRIVATE
    -ffunction-sections
    -fdata-sections
    -Wno-switch 
    -Wno-float-conversion 
    -Wno-double-promotion
    -fno-pic
    -fno-pie
)

target_sources(${APP} PRIVATE
    ${ROOT}/zephyr/src/arm_executor_runner.cpp
    ${ROOT}/zephyr/src/arm_memory_allocator.cpp
    ${ROOT}/zephyr/src/ethos_u_stub.cpp
    ${ROOT}/zephyr/src/register_quantized_ops.cpp
    ${ROOT}/et.cpp
  )

set_source_files_properties(
    ${ROOT}/zephyr/src/arm_executor_runner.cpp
    PROPERTIES
    COMPILE_DEFINITIONS ET_LOG_DUMP_OUTPUT
)


add_library(executorch_core STATIC IMPORTED)
set_target_properties(executorch_core PROPERTIES
  IMPORTED_LOCATION ${ROOT}/lib/libexecutorch_core.a
)

add_library(executorch STATIC IMPORTED)
set_target_properties(executorch PROPERTIES
  IMPORTED_LOCATION ${ROOT}/lib/libexecutorch.a
)

# Portable kernels and ops
if(EXISTS ${ROOT}/lib/libportable_kernels.a)
add_library(portable_kernels STATIC IMPORTED)
set_target_properties(portable_kernels PROPERTIES
  IMPORTED_LOCATION ${ROOT}/lib/libportable_kernels.a
)


endif() 

add_library(zephyr_ai_ops_lib STATIC IMPORTED)
set_target_properties(zephyr_ai_ops_lib PROPERTIES
  IMPORTED_LOCATION ${ROOT}/lib/libzephyr_ai_ops_lib.a
)


# Quantized kernels and ops
add_library(quantized_kernels STATIC IMPORTED)
set_target_properties(quantized_kernels PROPERTIES
  IMPORTED_LOCATION ${ROOT}/lib/libquantized_kernels.a
)


# Cortex-M optimized kernels (optional, for Cortex-M targets)
if(EXISTS ${ROOT}/lib/libcortex_m_kernels.a)
  add_library(cortex_m_kernels STATIC IMPORTED)
  set_target_properties(cortex_m_kernels PROPERTIES
    IMPORTED_LOCATION ${ROOT}/lib/libcortex_m_kernels.a
  )

endif()

# Ethos-U delegate (optional, for NPU acceleration)
if(EXISTS ${ROOT}/lib/libexecutorch_delegate_ethos_u.a)
  add_library(executorch_delegate_ethos_u STATIC IMPORTED)
  set_target_properties(executorch_delegate_ethos_u PROPERTIES
    IMPORTED_LOCATION ${ROOT}/lib/libexecutorch_delegate_ethos_u.a
  )
endif()


target_link_libraries(${APP} PRIVATE
  # Core ExecuTorch runtime
  executorch
  executorch_core

  # Quantized operators (used by most models)
  quantized_kernels
 
  zephyr_ai_ops_lib
)


if(EXISTS ${ROOT}/lib/libportable_kernels.a)
target_link_libraries(${APP} PRIVATE
  portable_kernels
)
endif()

# Add Cortex-M optimized kernels if available and targeting Cortex-M
# Note: These require CMSIS-NN library, so only link if it's available
if(CONFIG_CPU_CORTEX_M AND TARGET cortex_m_ops_lib AND TARGET CMSIS_NN)
  target_link_libraries(${APP} PRIVATE
    cortex_m_kernels
  )
endif()

# Add Ethos-U delegate if available
# The delegate contains static initializers that register the backend with ExecuTorch
if(EXISTS ${ROOT}/lib/libexecutorch_delegate_ethos_u.a)
  # Link the delegate library 
  target_link_libraries(${APP} PRIVATE
    ${ROOT}/lib/libexecutorch_delegate_ethos_u.a
    )
  
  # Link the object files directly to ensure static constructor is included
  # The static constructor _GLOBAL__sub_I_EthosUBackend.cpp registers the backend
  if(EXISTS ${ROOT}/lib/EthosUBackend.cpp.obj)
    target_link_libraries(${APP} PRIVATE
      ${ROOT}/lib/EthosUBackend.cpp.obj
      ${ROOT}/lib/VelaBinStream.cpp.obj
    )
  endif()
  
  # Link Zephyr's Ethos-U driver if available
  if(TARGET ethosu_core_driver)
    target_link_libraries(${APP} PRIVATE ethosu_core_driver)
  endif()
endif()

endfunction()