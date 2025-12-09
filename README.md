# Zephyr AI / Multimedia projects using CMSIS Stream

## Issues

Ethos driver init order must be changed in `zephyr\drivers\misc\ethos_u\ethos_u.c` and replaced with:

```C
	DEVICE_DT_INST_DEFINE(n, ethosu_zephyr_init, NULL, &ethosu_data_##n, &ethosu_dts_info_##n, \
			      APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY, NULL);
```

Otherwise a `malloc` may occur before the heap has been initialized and the demo is crashing at startup.


