# README

* cpp : The C++ template for the CMSIS Stream nodes in this projects
* c : Some C++ template rely on code in this C folder other use external libraries (CMSIS-DSP, TFLite ...)
* common : headers used by all app
* appa : A first app using CMSIS Stream

## appa

appa is a keyword spotting generated with ther python script `kws.py`

## common

The main application may want to interact with the running graph.
The mechanisms to interact with a graph are implemented with
`IdentifiedNode.hpp` that provides a C API where the implemented
interfaces can be checked at runtime.

Several default APIs may be supported in the project but a
given node may only implement a subset.

With the generated C API, it is possible to check if a node
implements an API at runtime and then interact with it.

All the complexities of the multiple-inheritance of C++ are
hidden and the node can be interacted with from C side.

`IdentifiedNode.hpp` should be customized to expose more interfaces if required by the application.

