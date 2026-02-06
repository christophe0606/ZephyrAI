# We hide all headers from external world because
# TFLite and executorch are using different and incompatible 
# versions of flatbuffer
add_library(executorch_runtime STATIC 
    runner/arm_executor_runner.cpp
    runner/arm_memory_allocator.cpp
    runner/arm_perf_monitor.cpp
)

target_compile_options(executorch_runtime PRIVATE -Wno-deprecated-declarations)

target_compile_definitions(executorch_runtime PRIVATE C10_USING_CUSTOM_GENERATED_MACROS
  ET_ARM_BAREMETAL_SCRATCH_TEMP_ALLOCATOR_POOL_SIZE=0x20000 
  ET_ARM_BAREMETAL_METHOD_ALLOCATOR_POOL_SIZE=0x8000
  ET_LOG_DUMP_OUTPUT
  ET_DEBUG_BUFFER_SIZE=0x8000     
)

target_include_directories(executorch_runtime PUBLIC  ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(executorch_runtime PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/libcortex_m_kernels.a
    ${CMAKE_CURRENT_LIST_DIR}/lib/libcortex_m_ops_lib.a
    ${CMAKE_CURRENT_LIST_DIR}/lib/libexecutorch_core.a
    "-Wl,--whole-archive"
    ${CMAKE_CURRENT_LIST_DIR}/lib/libexecutorch_delegate_ethos_u.a
    "-Wl,--no-whole-archive"
    ${CMAKE_CURRENT_LIST_DIR}/lib/libexecutorch.a
    ${CMAKE_CURRENT_LIST_DIR}/lib/libportable_kernels.a
    ${CMAKE_CURRENT_LIST_DIR}/lib/libportable_ops_lib.a
    ${CMAKE_CURRENT_LIST_DIR}/lib/libquantized_kernels.a
    "-Wl,--whole-archive"
    ${CMAKE_CURRENT_LIST_DIR}/lib/libquantized_ops_lib.a
    "-Wl,--no-whole-archive"
    )

target_include_directories(executorch_runtime PRIVATE include
     runner
     models
     include/executorch
     include/executorch/runtime
     include/executorch/runtime/core
     include/executorch/runtime/executor
     include/executorch/extension
     include/executorch/extension/data_loader
     include/executorch/extension/memory_allocator
     include/executorch/extension/runner_util
     include/executorch/util
     include/executorch/schema
     include/executorch/devtools
     include/executorch/devtools/etdump
     include/executorch/runtime/core/portable_type/c10
     include/executorch/kernels
     include/executorch/kernels/portable
     include/executorch/backends
     include/executorch/backends/xnnpack
     include/third-party
     include/third-party/flatbuffers
     include/third-party/flatbuffers/include)

