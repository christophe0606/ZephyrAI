#!/usr/bin/env bash
cd examples/arm/arm-scratch
toolchain_url="https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu/14.3.rel1/binrel/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi.tar.xz"
toolchain_dir="arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi"
curl --output "${toolchain_dir}.tar.xz" -L ${toolchain_url}

rm -rf "${toolchain_dir}"
tar xf "${toolchain_dir}.tar.xz"

toolchain_bin_path="$(cd ${toolchain_dir}/bin && pwd)"
echo "export PATH=${toolchain_bin_path}:\${PATH-}" >> setup_path.sh


