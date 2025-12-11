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
gain = Gain("gain",Q15_STEREO,NB,10)

deinterleave = DeinterleaveStereo("deinterleave",Q15_STEREO,NB)
to_f32 = Convert("to_f32",Q15_SCALAR,F32_SCALAR,NB)

audioWin=SlidingBuffer("audioWin",CType(F32),NB_WINDOW_SAMPLES,NB_OVERLAP_SAMPLES)
mfcc=MFCC("mfcc",NB_WINDOW_SAMPLES,MFCC_FEATURES)

mfccWin=SlidingBuffer("mfccWin",CType(F32),MFCC_FEATURES*NN_FEATURES,MFCC_FEATURES*MFCC_OVERLAP)

send = SendToNetwork("send",F32_SCALAR,MFCC_FEATURES*NN_FEATURES)

kws = KWS("kws",addr="GetModelPointer()",size="GetModelLen()")
display = KWSDisplay("display") 

classify = KWSClassify("classify")

nullRight = NullSink("nullRight",Q15_SCALAR,NB)


#the_graph.connect(src.o,gain.i)
#the_graph.connect(gain.o,deinterleave.i)

the_graph.connect(src.o,deinterleave.i)
the_graph.connect(deinterleave.l,to_f32.i)
the_graph.connect(to_f32.o,audioWin.i)
the_graph.connect(audioWin.o,mfcc.i)
the_graph.connect(mfcc.o,mfccWin.i)
the_graph.connect(mfccWin.o,send.i)
the_graph.connect(deinterleave.r,nullRight.i)

the_graph.connect(send["oev0"],kws["iev0"])
the_graph.connect(kws["oev0"],send["iev0"])
the_graph.connect(kws["oev1"],classify["iev0"])

the_graph.connect(classify["oev0"],display["iev0"])
#
conf = Configuration()
conf.CMSISDSP = False
conf.asynchronous = False
conf.horizontal=True
conf.nodeIdentification = True
conf.schedName = "scheduler"
conf.memoryOptimization = True
# Alif code is defining some variables as buf0, buf1 and not static
# They conflict with the buf0, buf1 defined by stream
# So a prefix is added 
conf.prefix = "stream"


scheduling = the_graph.computeSchedule(config=conf)

print("Schedule length = %d" % scheduling.scheduleLength)
print("Memory usage %d bytes" % scheduling.memory)

scheduling.ccode("src/stream/scheduler",conf)
scheduling.genJsonIdentification("src/stream./json",conf)
scheduling.genJsonSelectors("src/stream./json",conf)
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

subprocess.run(["dot","-Tpng","src/stream/scheduler/graph.dot","-o","src/stream/scheduler/graph.png"])