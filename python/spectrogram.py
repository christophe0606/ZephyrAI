from cmsis_stream.cg.scheduler import Graph,SlidingBuffer,CType,F32
from cmsis_stream.cg.scheduler.graphviz import Style

from .nodes import *
from .appnodes import *

from .generate import generate

def generate_spectrogram(codeSizeOptimization=False):
    the_graph = Graph()
    
    SAMPLING_FREQ_HZ = 16000
    AUDIO_PACKET_DURATION = 20 # ms (320 samples)
    OVERLAP_DURATION = AUDIO_PACKET_DURATION 
    WINDOWS_DURATION = 2*AUDIO_PACKET_DURATION 
    
    NB_AUDIO_SAMPLES = int(1e-3 * AUDIO_PACKET_DURATION * SAMPLING_FREQ_HZ)
    NB_OVERLAP_SAMPLES = int(1e-3 * OVERLAP_DURATION * SAMPLING_FREQ_HZ)
    NB_WINDOW_SAMPLES = int(1e-3 * WINDOWS_DURATION * SAMPLING_FREQ_HZ)
    
    FFT_SIZE = 1024 # 512
    
    NB = NB_AUDIO_SAMPLES
    
    #print(f"NB_AUDIO_SAMPLES={NB_AUDIO_SAMPLES}")
    #print(f"NB_OVERLAP_SAMPLES={NB_OVERLAP_SAMPLES}")
    #print(f"NB_WINDOW_SAMPLES={NB_WINDOW_SAMPLES}")
    
    # Use CMSIS VStream to connect to microphones
    #src = ZephyrDebugAudioSource("debugSource",NB)
    src = ZephyrAudioSource("audio",NB)
    gain = Gain("gain",Q15_STEREO,NB,4)
    to_f32 = Convert("to_f32",Q15_STEREO,F32_STEREO,NB)
    deinterleave = DeinterleaveStereo("deinterleave",F32_STEREO,NB)
    
    
    audioWinLeft=SlidingBuffer("audioWinLeft",CType(F32),NB_WINDOW_SAMPLES,NB_OVERLAP_SAMPLES)
    audioWinRight=SlidingBuffer("audioWinRight",CType(F32),NB_WINDOW_SAMPLES,NB_OVERLAP_SAMPLES)
    
    win_left = Hanning("winLeft",NB_WINDOW_SAMPLES,FFT_SIZE)
    win_right= Hanning("winRight",NB_WINDOW_SAMPLES,FFT_SIZE)
    
    to_complex_left = RealToComplex("toComplexLeft",F32,FFT_SIZE)
    to_complex_right= RealToComplex("toComplexRight",F32,FFT_SIZE)
    fft_left = CFFT("fftLeft",F32_COMPLEX,FFT_SIZE)
    fft_right = CFFT("fftRight",F32_COMPLEX,FFT_SIZE)
    
    spectrogram_left = Spectrogram("spectrogramLeft",FFT_SIZE)
    spectrogram_right= Spectrogram("spectrogramRight",FFT_SIZE)
    
    DISABLE_LEFT = False
    DISABLE_RIGHT = False
    
    nullSinkLeft = NullSink("nullSinkLeft",F32_COMPLEX,NB)
    nullSinkRight = NullSink("nullSinkRight",F32_SCALAR,NB)
    nullAll = NullSink("nullAll",Q15_STEREO,NB)
    
    display = SpectrogramDisplay("display")
    #display = DebugDisplay("display")
    
    
    if DISABLE_LEFT and DISABLE_RIGHT:
        the_graph.connect(src.o,nullAll.i)
    else:
        the_graph.connect(src.o,gain.i)
        the_graph.connect(gain.o,to_f32.i)
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
    
    class MyStyle(Style):
        
        def edge_color(self,edge):
            nb = self.fifoLength(edge) 
            if nb is None:
                nb = 0
            s = self.edgeSrcNode(edge)
            d = self.edgeDstNode(edge)
            
            if d.nodeName ==  "display":
               return("magenta")
            else: 
                if (nb > 512):
                    return("orange")
                return(super().edge_color(edge))
    
    generate("appb",the_graph,Style(),codeSizeOptimization=codeSizeOptimization)

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(
                    prog='kws',
                    description='Regenerate kws demo')
    parser.add_argument("--size", help="Code size optimization enabled", action='store_true')
    args = parser.parse_args()

    generate_spectrogram(codeSizeOptimization=args.size)
    if args.size:
        print("KWS demo generated with code size optimization")
        print("You need to call the generate script to regenerate the common files")
        print("shared between all applications.")