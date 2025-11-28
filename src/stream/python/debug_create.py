from cmsis_stream.cg.scheduler import Graph,Configuration

from nodes import *
from appnodes import *

    
the_graph = Graph()

AUDIO_BLOCK = 512 
FFT_SIZE = 512 

TEST = 2

if TEST == 1:
   src = DebugSource("audioSource",F32_SCALAR,AUDIO_BLOCK,frequency=1000,master=False)
   upsample = SRC("upsample",AUDIO_BLOCK)
   interleave = InterleaveStereo("interleave",F32_SCALAR,AUDIO_BLOCK*3)
   to_q15 = Convert("toQ15",F32_STEREO,Q15_STEREO,AUDIO_BLOCK*3)
   audioSink = VStreamAudioSink("audioSink",AUDIO_BLOCK*3,volume=60)

   the_graph.connect(src.o,upsample.i)
   the_graph.connect(upsample.o,interleave.l)
   the_graph.connect(upsample.o,interleave.r)
   the_graph.connect(interleave.o,to_q15.i)
   the_graph.connect(to_q15.o,audioSink.i)

if TEST == 2:
   #sine = DebugSource("sineWave",F32_SCALAR,AUDIO_BLOCK,frequency=1000)
   mic = VStreamAudioSource("mic",AUDIO_BLOCK,master=False)
   deinterleave = DeinterleaveStereo("deinterleave",F32_STEREO,AUDIO_BLOCK)
   to_f32 = Convert("toF32",Q15_STEREO,F32_STEREO,AUDIO_BLOCK)
   srcLeft = SRC("srcL",AUDIO_BLOCK)
   srcRight = SRC("srcR",AUDIO_BLOCK)
   interleave = InterleaveStereo("interleave",F32_SCALAR,AUDIO_BLOCK*3)
   to_q15 = Convert("toQ15",F32_STEREO,Q15_STEREO,AUDIO_BLOCK*3)
   audioSink = VStreamAudioSink("audioSink",AUDIO_BLOCK*3,volume=60)

   the_graph.connect(mic.o,to_f32.i)
   the_graph.connect(to_f32.o,deinterleave.i)  
   the_graph.connect(deinterleave.l,srcLeft.i)
   the_graph.connect(deinterleave.r,srcRight.i)
   the_graph.connect(srcLeft.o,interleave.l)
   the_graph.connect(srcRight.o,interleave.r)
   the_graph.connect(interleave.o,to_q15.i)
   the_graph.connect(to_q15.o,audioSink.i)

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

scheduling.ccode("../scheduler",conf)
scheduling.genJsonIdentification("../json",conf)
scheduling.genJsonSelectors("../json",conf)
scheduling.genJsonSelectorsInit("../json",conf)

def maybeFolder(x):
    if hasattr(x, "folder"):
        return x.folder + "/"
    # Standard ndoes from cmsis stream package have no folders
    return ""

with open("../scheduler/AppNodes.hpp","w") as f:
    #print(scheduling.allNodes)
    s = set([(maybeFolder(x),x.typeName) for x in scheduling.allNodes])
    for folder,n in s:
        if folder:
           print(f'#include "{folder}{n}.hpp"',file=f)

with open("../scheduler/graph.dot","w") as f:
    scheduling.graphviz(f)