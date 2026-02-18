cmake_minimum_required(VERSION 3.24)

project(executorch_embedded)

find_package(Python COMPONENTS Interpreter REQUIRED)
set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})

get_filename_component(EXECUTORCH_PARENT_DIR ${EXECUTORCH_DIR} DIRECTORY)
include_directories(${EXECUTORCH_PARENT_DIR})

set(EXECUTORCH_BUILD_EXECUTORCH_PYBIND OFF CACHE BOOL "" FORCE)
set(EXECUTORCH_BUILD_GENERATED_SRC OFF CACHE BOOL "" FORCE)
set(EXECUTORCH_BUILD_SDK OFF CACHE BOOL "" FORCE)

set(EXECUTORCH_BUILD_PORTABLE_OPS OFF CACHE BOOL "" FORCE)
  
set(EXECUTORCH_BUILD_EXECUTOR_RUNNER OFF CACHE BOOL "" FORCE)
set(EXECUTORCH_BUILD_EXTENSION_DATA_LOADER OFF CACHE BOOL "" FORCE)

set(EXECUTORCH_BUILD_CORTEX_M ON CACHE BOOL "" FORCE)
  
set(EXECUTORCH_BUILD_ARM_BAREMETAL ON CACHE BOOL "" FORCE)


set(EXECUTORCH_BUILD_EXTENSION_RUNNER_UTIL ON CACHE BOOL "" FORCE)
set(EXECUTORCH_BUILD_KERNELS_QUANTIZED ON CACHE BOOL "" FORCE)
set(EXECUTORCH_BUILD_KERNELS_QUANTIZED_AOT ON CACHE BOOL "" FORCE)


set(EXECUTORCH_ENABLE_LOGGING ON CACHE BOOL "" FORCE)
set(EXECUTORCH_ENABLE_EVENT_TRACER OFF CACHE BOOL "" FORCE)
set(EXECUTORCH_ENABLE_PROGRAM_VERIFICATION OFF CACHE BOOL "" FORCE)
set(EXECUTORCH_OPTIMIZE_SIZE ON CACHE BOOL "" FORCE)
set(EXECUTORCH_ENABLE_DTYPE_SELECTIVE_BUILD OFF CACHE BOOL "" FORCE)

add_subdirectory(${EXECUTORCH_DIR} ${CMAKE_CURRENT_BINARY_DIR}/executorch)



  # Only link portable_ops_lib if it was built (i.e., if portable ops are
  # enabled)
if(TARGET portable_ops_lib)
    message(STATUS "Linking with default portable_ops_lib")
else()
    message(
      STATUS
        "portable_ops_lib not available - using selective operators from application"
    )
endif()

#target_include_directories(
#    libexecutorch INTERFACE ${EXECUTORCH_DIR}/runtime
#                         ${EXECUTORCH_DIR}/third-party/flatcc/include
#)


set(ET_PTE_FILE_PATH ${CMAKE_CURRENT_BINARY_DIR}/../../model/ethos_u_minimal_example.pte)

execute_process(
  COMMAND
    ${PYTHON_EXECUTABLE} -m executorch.codegen.tools.gen_oplist 
    --model_file_path=${ET_PTE_FILE_PATH}
    --output_path=${CMAKE_CURRENT_BINARY_DIR}/temp.yaml
  OUTPUT_VARIABLE CMD_RESULT
)

execute_process(
  COMMAND
    ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/../../zephyr/scripts/select_ops_yaml_to_schema.py
    --input=${CMAKE_CURRENT_BINARY_DIR}/temp.yaml
    --output=${CMAKE_CURRENT_BINARY_DIR}/temp_schema.yaml
  OUTPUT_VARIABLE CMD_RESULT
)

# ${ET_PTE_FILE_PATH}


set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -fno-pic -fno-pie -Wall -Wno-switch -Wno-float-conversion -Wno-double-promotion -ffunction-sections -fdata-sections"
)

set(EXECUTORCH_ROOT ${EXECUTORCH_DIR})

if(NOT TARGET portable_kernels)
    set(EXECUTORCH_PORTABLE_BUILD_KERNELS_ONLY ON)
    add_subdirectory(
      ${EXECUTORCH_DIR}/kernels/portable
      ${CMAKE_CURRENT_BINARY_DIR}/executorch/kernels/portable
    )
    unset(EXECUTORCH_PORTABLE_BUILD_KERNELS_ONLY)
  endif()


include(${EXECUTORCH_DIR}/tools/cmake/Utils.cmake)
resolve_python_executable()
include(${EXECUTORCH_DIR}/tools/cmake/Codegen.cmake)

set(_functions_yaml "")
if(_functions_yaml STREQUAL "")
    set(_functions_yaml "${EXECUTORCH_DIR}/kernels/portable/functions.yaml")
endif()
  

set(_custom_ops_yaml "")
if(_custom_ops_yaml STREQUAL "")
  if(EXISTS "${EXECUTORCH_DIR}/kernels/quantized/quantized.yaml")
    set(_custom_ops_yaml "${EXECUTORCH_DIR}/kernels/quantized/quantized.yaml")
  else()
    set(_custom_ops_yaml "")
  endif()
endif()

set(_kernel_libs "")
  if(_kernel_libs STREQUAL "")
    set(_kernel_libs portable_kernels)
    if(TARGET quantized_kernels AND NOT _custom_ops_yaml STREQUAL "")
      list(APPEND _kernel_libs quantized_kernels)
    endif()
  endif()

  set(_deps "")
  if(_deps STREQUAL "")
    if(TARGET executorch)
      set(_deps executorch)
    else()
      set(_deps executorch_core)
    endif()
  endif()

  set(_include_all_ops FALSE)
  
set(_ops_from_model "")

gen_selected_ops(
    LIB_NAME "zephyr_ai_ops_lib"
    OPS_SCHEMA_YAML ${CMAKE_CURRENT_BINARY_DIR}/temp_schema.yaml
    #ROOT_OPS "${EXECUTORCH_SELECT_OPS_LIST}"
    INCLUDE_ALL_OPS ${_include_all_ops}
    OPS_FROM_MODEL "${_ops_from_model}"
    #DTYPE_SELECTIVE_BUILD "${EXECUTORCH_ENABLE_DTYPE_SELECTIVE_BUILD}"
  )

  generate_bindings_for_kernels(
    LIB_NAME "zephyr_ai_ops_lib" 
    FUNCTIONS_YAML "${_functions_yaml}"
    CUSTOM_OPS_YAML ${_custom_ops_yaml}
    #DTYPE_SELECTIVE_BUILD "${EXECUTORCH_ENABLE_DTYPE_SELECTIVE_BUILD}"
  )
  gen_operators_lib(
    LIB_NAME "zephyr_ai_ops_lib"
    KERNEL_LIBS "${_kernel_libs}"
    DEPS "${_deps}"
    #DTYPE_SELECTIVE_BUILD "${EXECUTORCH_ENABLE_DTYPE_SELECTIVE_BUILD}"
  )


set(_local_flatcc_root ${CMAKE_BINARY_DIR}/flatcc_src)
if(NOT EXISTS ${_local_flatcc_root}/CMakeLists.txt)
  file(MAKE_DIRECTORY ${_local_flatcc_root})
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${EXECUTORCH_DIR}/third-party/flatcc ${_local_flatcc_root}
  )
endif()
set(EXECUTORCH_FLATCC_SOURCE_ROOT
    ${_local_flatcc_root}
    CACHE PATH "" FORCE
)
set(EXECUTORCH_FLATCC_INSTALL_ROOT
    ${_local_flatcc_root}
    CACHE PATH "" FORCE
)

