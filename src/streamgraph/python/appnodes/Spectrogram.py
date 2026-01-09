from cmsis_stream.cg.scheduler import GenericSink
from nodes import *

    
class Spectrogram(GenericSink):
    def __init__(self,name,nbSamples):
        GenericSink.__init__(self,name)
        self.addInput("i",F32_COMPLEX,nbSamples)
        self.addEventOutput()

    @property
    def folder(self):
        """The folder where the C++ implementation of this node is located"""
        return "appnodes"
    
    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "Spectrogram"