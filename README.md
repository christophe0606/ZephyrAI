# Zephyr AI / Multimedia projects using CMSIS Stream

Example of use of CMSIS Stream to implement AI + audio + video apps on Alif E7 board.

This version uses the CMSIS Stream Zephyr module.

You can add it to your west file with:
```
- name: cmsisstream
  url: https://github.com/ARM-software/CMSIS-Stream
  revision: main
  path: modules/lib/cmsisstream
```

The Python script `src\streamgraph\python\kws.py` can be used
to generate the KWS example.

You need version at least `3.0.0` of the CMSIS Stream Python package.


Other Python scripts in `src\streamgraph\python` have not yet been
updated to use the new directory structure.

## Next steps
* Context switching between graphs
* Code size optimization (for C++ template when used in several graphs)

# Build issues
Use sdk-alif Commit  1e1d38883a59e121592c1f23f3d4185453587cbe of
https://github.com/alifsemi/sdk-alif is used

Display problems with more recent version.

But there are issues with this old commit and they need to be solved first. See below.


## Issues

Ethos driver init order must be changed in `zephyr\drivers\misc\ethos_u\ethos_u.c` and replaced with:

```C
	DEVICE_DT_INST_DEFINE(n, ethosu_zephyr_init, NULL, &ethosu_data_##n, &ethosu_dts_info_##n, \
			      APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY, NULL);
```

Otherwise a `malloc` may occur before the heap has been initialized and the demo is crashing at startup.
It is probably an error of configuration of this project ?


Using `APPLICATION` level is not the right way (and deprecated) but I don't want to have to change the driver too much.


Zephyr cmakefile for cmsis-dsp module must be changed
(since Alif version not using the latest Zephyr yet in this old commit).

`  zephyr_library_compile_definitions(ZEPHYR_INCLUDE_TOOLCHAIN_STDINT_H_)
` must be added before the `zephyr_library_compile_definitions_ifdef`
to be able to compile with Helium.

