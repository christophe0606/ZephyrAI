from cmsis_stream.cg.scheduler import GenericNode,CType,Q15
from .NodeTypes import *

class SpeexPreprocess(GenericNode):
    def __init__(self,name):
        GenericNode.__init__(self,name)
        # Size of FFT is hardcoded into spx_fft_init
        self.addInput("i",CType(Q15),256)
        self.addOutput("o",CType(Q15),256)

    @property
    def typeName(self):
        return "SpeexPreprocess"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "nodes"