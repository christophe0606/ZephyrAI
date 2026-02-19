REM west build -p auto -d out/Zephyr_M55_HE/AppKit-E7/Debug -b alif_e7_dk/ae722f80f55d5xx/rtss_he .

west build -p auto -b  alif_e7_dk_rtss_he -d out/Zephyr_M55_HE/AppKit-E7/Debug . 

west build -t menuconfig -d out/Zephyr_M55_HE/AppKit-E7/Debug
west build -t ram_report -d out/Zephyr_M55_HE/AppKit-E7/Debug

REM west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he . ^
REM  -DCONFIG_DEBUG=y -DZEPHYR_TOOLCHAIN_VARIANT="llvm" ^
REM  -DLLVM_TOOLCHAIN_PATH="C:/Users/chrfav01/.vcpkg/artifacts/2139c4c6/compilers.arm.llvm.embedded/21.1.0"

REM west build -p auto -b alif_e7_dk_rtss_he .  -DCMAKE_BUILD_TYPE=RelWithDebInfo

REM python zephyr-tools\analyse_map_file.py out\Zephyr_M55_HE\AppKit-E7\Debug\zephyr\zephyr.map  --mode ram --html ram_report.html