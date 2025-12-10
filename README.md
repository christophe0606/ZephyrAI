# Zephyr AI / Multimedia projects using CMSIS Stream

## Issues

Ethos driver init order must be changed in `zephyr\drivers\misc\ethos_u\ethos_u.c` and replaced with:

```C
	DEVICE_DT_INST_DEFINE(n, ethosu_zephyr_init, NULL, &ethosu_data_##n, &ethosu_dts_info_##n, \
			      APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY, NULL);
```

Otherwise a `malloc` may occur before the heap has been initialized and the demo is crashing at startup.


Using `APPLICATION` level is not the right way (and deprecated) but I don't want to have to change the driver too much.


Zephyr cmakefile for cmsis-dsp module must be changed
(since Alif version not using the latest Zephyr yet).

`  zephyr_library_compile_definitions(ZEPHYR_INCLUDE_TOOLCHAIN_STDINT_H_)
` must be added before the `zephyr_library_compile_definitions_ifdef`
to be able to compile with Helium.