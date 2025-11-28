from cmsis_stream.cg.scheduler import Graph,Configuration
from cmsis_stream.cg.scheduler.graphviz import Style

from nodes import *
from appnodes import *

    
the_graph = Graph()

AUDIO_BLOCK = 512 
FFT_SIZE = 512 

# Use CMSIS VStream to connect to microphones
src = VStreamAudioSource("audioSource",AUDIO_BLOCK)
# With video there are underflow or overflow in audio
# vStream may not be flexible enough and using SAI 
# directly may be better to support more
# constrained environment
#speaker = VStreamAudioSink("audioSink",3*AUDIO_BLOCK)
speaker = NullSink("audioSink",Q15_STEREO,3*AUDIO_BLOCK)
video = VStreamVideoSource("videoSource")

src_left = SRC("srcLeft",AUDIO_BLOCK)
src_right= SRC("srcRight",AUDIO_BLOCK)

# Debug source can be used instead to generate a sine
# with amplitude modulation
#src = DebugSource("audioSource",AUDIO_BLOCK)
to_f32 = Convert("to_f32",Q15_STEREO,F32_STEREO,AUDIO_BLOCK)
to_q15 = Convert("to_q15",F32_STEREO,Q15_STEREO,3*AUDIO_BLOCK)

win_left = Hanning("winLeft",AUDIO_BLOCK)
win_right= Hanning("winRight",AUDIO_BLOCK)

deinterleave = DeinterleaveStereo("deinterleave",F32,AUDIO_BLOCK)
interleave = InterleaveStereo("interleave",F32,3*AUDIO_BLOCK)

to_complex_left = RealToComplex("toComplexLeft",F32,AUDIO_BLOCK)
to_complex_right= RealToComplex("toComplexRight",F32,AUDIO_BLOCK)
fft_left = CFFT("fftLeft",F32_COMPLEX,FFT_SIZE)
fft_right = CFFT("fftRight",F32_COMPLEX,FFT_SIZE)

spectrogram_left = Spectrogram("spectrogramLeft",FFT_SIZE)
spectrogram_right= Spectrogram("spectrogramRight",FFT_SIZE)

display = AppDisplay("display")

mixer = Mixer("mixer",AUDIO_BLOCK)


the_graph.connect(src.o,to_f32.i)
the_graph.connect(to_f32.o,deinterleave.i)
the_graph.connect(deinterleave.l,win_left.i)
the_graph.connect(deinterleave.r,win_right.i)

the_graph.connect(win_left.o,to_complex_left.i)
the_graph.connect(win_right.o,to_complex_right.i)

the_graph.connect(to_complex_left.o,fft_left.i)
the_graph.connect(to_complex_right.o,fft_right.i)
the_graph.connect(fft_left.o,spectrogram_left.i)
the_graph.connect(fft_right.o,spectrogram_right.i)

the_graph.connect(spectrogram_left["oev0"],display["iev0"])
the_graph.connect(spectrogram_right["oev0"],display["iev1"])
the_graph.connect(video["oev0"],display["iev2"])

the_graph.connect(deinterleave.l,mixer.inl)
the_graph.connect(deinterleave.r,mixer.inr)

the_graph.connect(mixer.oul,src_left.i)
the_graph.connect(mixer.our,src_right.i)

the_graph.connect(src_left.o,interleave.l)
the_graph.connect(src_right.o,interleave.r)
the_graph.connect(interleave.o,to_q15.i)
the_graph.connect(to_q15.o,speaker.i)

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

with open("../scheduler/graph.dot","w") as f:
    scheduling.graphviz(f,style=myStyle)