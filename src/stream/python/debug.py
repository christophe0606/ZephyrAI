from cmsis_stream.cg.scheduler import Graph,Configuration,SlidingBuffer,CType,F32
from cmsis_stream.cg.scheduler.graphviz import Style

from nodes import *
from appnodes import *

import subprocess

the_graph = Graph()

SAMPLING_FREQ_HZ = 16000
AUDIO_PACKET_DURATION = 16 # ms (256 samples)
OVERLAP_DURATION = 16 
WINDOWS_DURATION = 32 

NB_AUDIO_SAMPLES = int(1e-3 * AUDIO_PACKET_DURATION * SAMPLING_FREQ_HZ)
NB_OVERLAP_SAMPLES = int(1e-3 * OVERLAP_DURATION * SAMPLING_FREQ_HZ)
NB_WINDOW_SAMPLES = int(1e-3 * WINDOWS_DURATION * SAMPLING_FREQ_HZ)

FFT_SIZE = NB_WINDOW_SAMPLES # 512

NB = NB_AUDIO_SAMPLES

print(f"NB_AUDIO_SAMPLES={NB_AUDIO_SAMPLES}")
print(f"NB_OVERLAP_SAMPLES={NB_OVERLAP_SAMPLES}")
print(f"NB_WINDOW_SAMPLES={NB_WINDOW_SAMPLES}")

# Use CMSIS VStream to connect to microphones
#src = ZephyrDebugAudioSource("debugSource",NB)
src = ZephyrAudioSource("audio",NB)
to_f32 = Convert("to_f32",Q15_STEREO,F32_STEREO,NB)
deinterleave = DeinterleaveStereo("deinterleave",F32_STEREO,NB)


audioWinLeft=SlidingBuffer("audioWinLeft",CType(F32),NB_WINDOW_SAMPLES,NB_OVERLAP_SAMPLES)
audioWinRight=SlidingBuffer("audioWinRight",CType(F32),NB_WINDOW_SAMPLES,NB_OVERLAP_SAMPLES)

win_left = Hanning("winLeft",NB_WINDOW_SAMPLES)
win_right= Hanning("winRight",NB_WINDOW_SAMPLES)

to_complex_left = RealToComplex("toComplexLeft",F32,NB_WINDOW_SAMPLES)
to_complex_right= RealToComplex("toComplexRight",F32,NB_WINDOW_SAMPLES)
fft_left = CFFT("fftLeft",F32_COMPLEX,FFT_SIZE)
fft_right = CFFT("fftRight",F32_COMPLEX,FFT_SIZE)

spectrogram_left = Spectrogram("spectrogramLeft",FFT_SIZE)
spectrogram_right= Spectrogram("spectrogramRight",FFT_SIZE)

DISABLE_LEFT = False
DISABLE_RIGHT = True

nullSinkLeft = NullSink("nullSinkLeft",F32_COMPLEX,NB)
nullSinkRight = NullSink("nullSinkRight",F32_SCALAR,NB)
nullAll = NullSink("nullAll",Q15_STEREO,NB)

display = AppDisplay("display")
#display = DebugDisplay("display")


if DISABLE_LEFT and DISABLE_RIGHT:
    the_graph.connect(src.o,nullAll.i)
else:
    the_graph.connect(src.o,to_f32.i)
    the_graph.connect(to_f32.o,deinterleave.i)
    if DISABLE_LEFT:
        the_graph.connect(deinterleave.l,nullSinkLeft.i)
    else:
        the_graph.connect(deinterleave.l,audioWinLeft.i)
        the_graph.connect(audioWinLeft.o,win_left.i)
        the_graph.connect(win_left.o,to_complex_left.i)
        the_graph.connect(to_complex_left.o,fft_left.i)
        the_graph.connect(fft_left.o,spectrogram_left.i)
        the_graph.connect(spectrogram_left["oev0"],display["iev0"])
    
    if DISABLE_RIGHT:
        the_graph.connect(deinterleave.r,nullSinkRight.i)
    else:
        the_graph.connect(deinterleave.r,audioWinRight.i)
        the_graph.connect(audioWinRight.o,win_right.i)
        the_graph.connect(win_right.o,to_complex_right.i)
        the_graph.connect(to_complex_right.o,fft_right.i)
        the_graph.connect(fft_right.o,spectrogram_right.i)
        the_graph.connect(spectrogram_right["oev0"],display["iev1"])

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