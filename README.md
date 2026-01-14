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

The Python script `src/streamgraph/python/kws.py` can be used
to generate the KWS example in folder `src/streamgraph/appa`

The Python script `src/streamgraph/python/spectrogram.py` can be used
to generate the KWS example in folder `src/streamgraph/appb`

You need version at least `3.0.0` of the CMSIS Stream Python package. It was recently updated so do a:

```
pip install cmsis-stream --upgrade
```


Other Python scripts in `src\streamgraph\python` have not yet been
updated to use the new directory structure.

In current version of the demo, only appa is launched.

## Next steps
* Context switching between graphs

# Code size optimizations

With several CMSIS Stream implementing several applications, a C++ template may be instantiated several times : in each graph.

The linker may be able to remove the redundant definitions. But since it is compiler dependent, the demo provides the possibility to move the template instantiations to a specific unique `cpp` file.

To enable this code optimizations you need:
* Generate the graphs by passing the `--size` option

It will add several `extern template` so that the templates are not instantiated in each scheduler.

Then, you need to call the `generate.py` script and list all the apps:
```python
src/streamgraph/python/generate.py appa appb
```

It will generate `src/streamgraph/common/template_instantiations.cpp`. 

This file instantiates the C++ template used in all the graphs without duplication.

Finally, this file must be included in the build by changing an option in `prj.conf`:

`CONFIG_TEMPLATE_INSTANTIATIONS=y`

# Build issues
Use sdk-alif Commit  1e1d38883a59e121592c1f23f3d4185453587cbe of
https://github.com/alifsemi/sdk-alif 

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

