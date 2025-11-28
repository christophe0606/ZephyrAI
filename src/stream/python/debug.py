from cmsis_stream.cg.scheduler import Graph,Configuration,SlidingBuffer,CType,F32
from cmsis_stream.cg.scheduler.graphviz import Style

from nodes import *
from appnodes import *

import subprocess

the_graph = Graph()

SAMPLING_FREQ_HZ = 16000
AUDIO_PACKET_DURATION = 20 # ms 
OVERLAP_DURATION = 20 
WINDOWS_DURATION = 40 

NB_AUDIO_SAMPLES = int(1e-3 * AUDIO_PACKET_DURATION * SAMPLING_FREQ_HZ)
NB_OVERLAP_SAMPLES = int(1e-3 * OVERLAP_DURATION * SAMPLING_FREQ_HZ)
NB_WINDOW_SAMPLES = int(1e-3 * WINDOWS_DURATION * SAMPLING_FREQ_HZ)


NB = NB_AUDIO_SAMPLES
MFCC_FEATURES = 10
NN_FEATURES = 49

# Every new "audio" block of 20ms a new full tensor input is generated
# If it is too often, the overlap can be decreased
MFCC_OVERLAP = NN_FEATURES-1

# Use CMSIS VStream to connect to microphones
src = ZephyrAudioSource("audioSource",NB)
deinterleave = DeinterleaveStereo("deinterleave",Q15_STEREO,NB)
convert = StereoToMono("stereoToMono",Q15_SCALAR,NB)

nullSink = NullSink("nullSink",Q15_SCALAR,NB)


the_graph.connect(src.o,deinterleave.i)
the_graph.connect(deinterleave.l,convert.l)
the_graph.connect(deinterleave.r,convert.r)
the_graph.connect(convert.o,nullSink.i)


#
conf = Configuration()
conf.CMSISDSP = False
conf.asynchronous = False
conf.horizontal=True
conf.nodeIdentification = True
conf.schedName = "scheduler"
conf.memoryOptimization = True

scheduling = the_graph.computeSchedule(config=conf)

print("Schedule length = %d" % scheduling.scheduleLength)
print("Memory usage %d bytes" % scheduling.memory)

scheduling.ccode("src/stream/scheduler",conf)
scheduling.genJsonIdentification("src/stream/json",conf)
scheduling.genJsonSelectors("src/stream/json",conf)
scheduling.genJsonSelectorsInit("src/stream/json",conf)

def maybeFolder(x):
    if hasattr(x, "folder"):
        return x.folder + "/"
    # Standard ndoes from cmsis stream package have no folders
    return ""

with open("src/stream/scheduler/AppNodes.hpp","w") as f:
    #print(scheduling.allNodes)
    s = set([(maybeFolder(x),x.typeName) for x in scheduling.allNodes])
    for folder,n in s:
        if folder:
           print(f'#include "{folder}{n}.hpp"',file=f)


class MyStyle(Style):
    
    def edge_color(self,edge):
        nb = self.fifoLength(edge) 
        s = self.edgeSrcNode(edge)
        d = self.edgeDstNode(edge)
        
        if d.nodeName ==  "display":
           return("magenta")
        else: 
            if (nb > 512):
                return("orange")
            return(super().edge_color(edge))

myStyle = MyStyle()

with open("src/stream/scheduler/graph.dot","w") as f:
    scheduling.graphviz(f)

subprocess.run(["dot","-Tpng","src/stream/scheduler/graph.dot","-o","src/stream/scheduler/kws.png"])