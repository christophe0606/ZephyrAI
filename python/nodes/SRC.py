from cmsis_stream.cg.scheduler import GenericNode
from .NodeTypes import *


class SRC(GenericNode):
    def __init__(self,name,length,inputFreq=16,outputFreq=48):
        GenericNode.__init__(self,name,identified=False)
        if inputFreq != 16 and outputFreq != 48:
            raise Exception("Current C++ implementation of SRC only supports 16kHz to 48kHz conversion")
        self.addInput("i",F32_SCALAR,length)
        self.addOutput("o",F32_SCALAR,3*length)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "SRC"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "nodes"