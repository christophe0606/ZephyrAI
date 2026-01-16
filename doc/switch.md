# Context switching

The CMSIS Stream Zephyr module provides a tentative API to implement context switching between graphs.

The assumptions are:

* All graphs are in memory. One is running. Other are paused
* Threads are reused to execute different graphs
* Context switching is cooperative
* HW peripherals may be shared between graphs and are initialized outside of graphs.
