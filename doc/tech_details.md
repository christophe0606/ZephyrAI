# Technical details about the demo

Some explanation about the APIs exposed by the Zephyr CMSIS Stream module and the structure of the applications.

## HW Configuration

At the entry of `main.cpp`, the hardware peripherals used in the demo are configured. The peripheral are selected depending on options enabled in the `prj.conf` module.

In this demo we only have input microphone and display. 

## Parameter setting for the nodes

Then, we initialize the parameters of the nodes in each graph.

Most are already initialized in files `src\streamgraph\appa\appa_params.c` and `src\streamgraph\appa\appb_params.c`.

Those files could be generated from a `yaml` file.

But for the TensorFlow Lite for Micro (TFLM) node, we need to pass the pointer to the model and the size. It is hardcoded in `main.cpp` because we have not yet defined any convention about how the model information should be passed to the TFLM nodes in the network from a yaml file.

Then we initialize in a generic way the information related to hardware peripheral.

Each parameter structure is beginning with a field named `hw_` (underscore is to avoid conflict with the names of the node) and of type `hardwareParams`.

With this convention, we can initialize the hardware peripheral params although each parameter structure is of a different type. But they can all be casted to `hardwareParams`.

## Stream initialization

Then, CMSIS Stream and the graphs must be initialized.

We call `stream_init_memory` that is mainly allocating some memory pools used by the CMSIS Stream event system.

Then, we create an event queue per graph.

Then we initialize each graph with its event queue and its parameters. The parameters are used by each node to set some settings. The event queue is used by some nodes which need to send events.

This can't be generic because each graph use a different parameter structure. As consequence, if you need to add new applications to the demo, you'll need to add new initializations.

Then, the demo is initializing an "interrupt" thread. It is a bit a miscellaneous thread for purpose is to translate some event occurring in the RTOS to CMSIS Stream actions. Here we translate the `switch` command to a context switch. One could also send CMSIS Events in reaction to interrupts etc ...

Then we initialize some CMSIS Stream execution context. It is a datatype defined by the CMSIS Stream Zephyr module. It is used for the context switch and contain all the information needed.

Finally, we start the CMSIS Stream execution using one context with `stream_start_threads`.

This function starts two threads : thread to run the dataflow graph. It has the higher priority.

A thread to execute events from the event queue. It can switch between 3 different priorities but they should be less than the dataflow one since dataflow is generally used for the processing the most constrained by real-time (like audio).

The, the call to `stream_wait_for_threads_end` will block until both threads have stopped.

## Context switching

The CMSIS Stream Zephyr module provides a tentative API to implement context switching between graphs.

The assumptions are:

* All graphs are in memory. One is running. Other are paused
* Threads are reused to execute different graphs
* Context switching is cooperative
* HW peripherals may be shared between graphs and are initialized outside of graphs.
* Some dataflow nodes must exist. For a pure event graph, context switching is not yet fully supported

### All graphs in memory

If the graphs were not all in memory, we would have to destroy / re-create graphs at each context switch. It would increase the latency of the context switch but it would also cause trouble with memory fragmentation.

A graph is very general with node than can be anything. As consequence, the memory allocator also have to be general.

With all graphs in memory we have the other problem of memory consumption:

* Display buffers are shared between the graphs
* Audio memory slab is shared between graphs
* Tensor arena is shared between graphs
* CMSIS FIFOs could be shared by using memory overlays at linker level since the memory sectino for each graph are never used at same timer and FIFO are cleared when switching

So, it looks like the big consumer of memory are not a problem. If some other nodes are also allocating lot of memory then we should look at how it can be shared between graphs

## Thread reuse

We do not need to run several graphs at same time. So the Zephyr module is assuming only one graph is running a a given time and reuse the same thread

## Cooperative context switching



Context switching between graph only occurs when a node has finished executing its dataflow processing or its events.

It make the switching easier since it occurs only at specific points and it does not impose any constraint on the drivers.

Since we are targeting real time application, each processing should not take a lot of time and should not delay the context switch for too long.

If there is a deadlock then it is not only the context switch that is broken but the application too.

So cooperative context switching is not a problem for the use case we target.

### HW peripherals

HW peripherals are configured outside of the graphs. That's why we have a common `hw_` filed in all parameter structures.

We context switching a node can just disable / enable interrupts for a given peripheral.

In this demo we do not change settings like sampling rate etc .. when switching between graphs. It may be added and there is not limitation.

## Context switch procedure in details

There are two threads to execute a CMSIS Stream graph:

* The data flow thread
* The event thread

Context switching is implemented by sending RTOS events to the thread to pause / restart them and waiting for RTOS events sent by the thread to confirm they are entering a pause mode.

If you look at the interrupt thread which is responsible for doing the context switch in `main.cpp`, you'll see



```cpp
stream_pause_current_scheduler();
stream_resume_scheduler(&contexts[currentNetwork]);
```

First, the CMSIS Stream thread are paused. The function is blocking and wait for confirmation from CMSIS Stream that the threads have been paused.

Then, we resume the threads with a different execution context.

During the pause / resume, the function will call the pause / resume from the execution context.

The pause / resume functions are provided by `main.cpp`. We scan the graph to know which nodes are implementing the `ContextSwitch` interface. If the interface is implement, the `pause` or `resume` function is called.

Some node may use this to disable / enable interrupts from an HW peripheral. Some other nodes may use this to clear internal state or memory.

Pausing the event thread  in the CMSIS Stream Zephyr module is done by pausing the event queue.

Pausing the event queue means:

* The event queue is no more accepting any event from the graph 
* The event queue processing loop is finished and  return the processing to the main code in the event thread. The event thread sends and RTOS event to signal it is pausing.

Then the `stream_pause_current_scheduler` is cleaning all events remaining in the event queue.

Pausing the data flow stream is done by sending an RTOS event to the thread. The data flow scheduler is returning and the data flow thread is paused.



Resuming an execution with a new context means:

* We store the new current context
* The new event queue is resumed : it means it can now accept events from a graph
* We wake up the data flow thread that will start executing the dataflow scheduler from current context
* We wake up the event thread that will enter the event queue processing loop

So context switching only occurs when:

* Both threads are paused
* Event queue has been cleaned of all events



When data flow graph is paused:

* All FIFOs are reset to their initial state and buffer cleaned to zero.
* `pause` function called on all graphs implementing it

When data flow graph is resumed:

* `resume` function called on all graphs implementing it



In case of memory overlay of FIFOs for different graphs:

* Resetting FIFOs should occur during resume
* If there is no data flow graph, the data flow thread will go into pause automatically and it is not right since nodes with a `pause` function will be paused although the full graph is not paused (events are still enabled).

We need to export an additional information (schedule length) from the Python script and it is simpler if it is done from the CMSIS Stream Python package. So it will have to be updated.

If it was done from the demo, a new header would have to be generated with this information.

So context switching needs some improvement.



