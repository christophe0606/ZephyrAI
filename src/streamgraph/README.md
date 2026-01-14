# README

* streamnodes : The C++ templates for the CMSIS Stream nodes in the applications
* nodes_csrc : Some C++ templates rely on code in this C folder and other use external libraries (CMSIS-DSP, TFLite ...)
* common : headers used by all app
* appa : A first app using CMSIS Stream
* appb : A second app using CMSIS Stream

Current version of the demo only launch appa.

## appa

appa is a keyword spotting demo using audio from microphone.
It is generated with the python script `kws.py` from root folder

```python
python ./src/streamgraph/python/kws.py
```

## appb

appb displays a stereo spectrogram on display. The audio comes from the microphone.
It is generated with the python script `spectrogram.py`

```python
python ./src/streamgraph/python/spectrogram.py
```


## common

### `app_config.h`

It is a global configuration file for all the apps. 

### `rtos_events.hpp`

Event ID definitions that can be used to send RTOS events to the CMSIS stream data flow thread.
(Should not be confused with CMSIS Stream events).
Those events could be used to pause / stop the data flow.

### `template_instantiations.cpp`

With some build settings, this file is built and is used to instantiate the C++ templates in only one place.

### Other files

Other files are used to provided several C APIs to interact with the graph.

The main application may want to interact with the running graph.
The mechanisms to interact with a graph are implemented with
`IdentifiedNode.hpp` that provides several C APIs where the implemented
interfaces can be checked at runtime.

Several APIs may be supported in the project but a
given node may only implement a subset.

With the generated C API, it is possible to check if a node
implements an API at runtime and then interact with it.

All the complexities of the multiple-inheritance of C++ are
hidden and the node can be interacted with from C side.

`IdentifiedNode.hpp` and `cstream_node.h` should be customized to expose more interfaces if required by the application.

