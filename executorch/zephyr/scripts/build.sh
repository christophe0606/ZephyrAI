#!/usr/bin/env bash

cd /workspace2

source /workspace/executorch-venv/bin/activate


python zephyr/scripts/pte_to_header.py --pte \
    model/ethos_u_minimal_example.pte --outdir out/build_bin --section \
    CONFIG_ET_MODEL

cp out/build_bin/model_pte.h model/model_pte.h

mkdir -p out/build 
mkdir -p out/build_bin
cp zephyr/.docker/CMakeLists.txt out/build/

cmake --fresh -S out/build -B out/build_bin -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=Release \
  -DEXECUTORCH_DIR="/workspace/executorch" \
  -DCMAKE_TOOLCHAIN_FILE="/workspace2/model/arm-none-eabi-gcc.cmake"

cmake --build out/build_bin

cp out/build_bin/executorch/backends/arm/libexecutorch_delegate_ethos_u.a lib/

cp out/build_bin/executorch/backends/cortex_m/libcortex_m_kernels.a lib/
##cp out/build_bin/executorch/backends/cortex_m/libcortex_m_ops_lib.a lib/

cp out/build_bin/executorch/kernels/portable/libportable_kernels.a lib/
#cp out/build_bin/executorch/kernels/portable/libportable_ops_lib.a lib/

cp out/build_bin/executorch/libexecutorch.a lib/
cp out/build_bin/executorch/libexecutorch_core.a lib/

cp out/build_bin/executorch/kernels/quantized/libquantized_kernels.a lib/
#cp out/build_bin/executorch/kernels/quantized/libquantized_ops_lib.a lib/

cp out/build_bin/libzephyr_ai_ops_lib.a lib/

# zephyr_ai_ops_lib
# [ -f file ] && 

/workspace/executorch/examples/arm/arm-scratch/arm-gnu-toolchain-13.3.rel1-arm-none-eabi/bin/arm-none-eabi-ar x lib/libexecutorch_delegate_ethos_u.a EthosUBackend.cpp.obj VelaBinStream.cpp.obj
ar x lib/libexecutorch_delegate_ethos_u.a EthosUBackend.cpp.obj VelaBinStream.cpp.obj 
cp EthosUBackend.cpp.obj lib/
cp VelaBinStream.cpp.obj lib/
rm EthosUBackend.cpp.obj VelaBinStream.cpp.obj

cp  out/build_bin/zephyr_ai_ops_lib/*.h include/executorch/generated/
